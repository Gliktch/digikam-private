/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * SPDX-FileCopyrightText: 2018      by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * SPDX-FileCopyrightText: 2018-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ipfstalker.h"

// Qt includes

#include <QUrl>
#include <QFile>
#include <QQueue>
#include <QUrlQuery>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "networkmanager.h"

namespace DigikamGenericIpfsPlugin
{

class Q_DECL_HIDDEN IpfsTalker::Private
{
public:

    explicit Private()
      : ipfsUploadUrl(QLatin1String("https://api.globalupload.io/transport/add")),
        workTimer    (0),
        reply        (nullptr),
        image        (nullptr),
        netMngr      (nullptr)
    {
    }

    // The ipfs upload url

    const QString            ipfsUploadUrl;

    // Work queue

    QQueue<IpfsTalkerAction> workQueue;

    // ID of timer triggering on idle (0ms)

    int                      workTimer;

    // Current QNetworkReply

    QNetworkReply*           reply;

    // Current image being uploaded

    QFile*                   image;

    // The QNetworkAccessManager used for connections

    QNetworkAccessManager*   netMngr;
};

IpfsTalker::IpfsTalker(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);
}

IpfsTalker::~IpfsTalker()
{
    // Disconnect all signals as cancelAllWork may emit

    disconnect(this, nullptr, nullptr, nullptr);
    cancelAllWork();

    delete d;
}

unsigned int IpfsTalker::workQueueLength()
{
    return d->workQueue.size();
}

void IpfsTalker::queueWork(const IpfsTalkerAction& action)
{
    d->workQueue.enqueue(action);
    startWorkTimer();
}

void IpfsTalker::cancelAllWork()
{
    stopWorkTimer();

    if (d->reply)
    {
        d->reply->abort();
    }

    // Should error be emitted for those actions?

    while (!d->workQueue.empty())
    {
        d->workQueue.dequeue();
    }
}

void IpfsTalker::uploadProgress(qint64 sent, qint64 total)
{
    if (total > 0) // Don't divide by 0
    {
        Q_EMIT progress((sent * 100) / total, d->workQueue.first());
    }
}

void IpfsTalker::replyFinished()
{
    auto* reply = d->reply;
    reply->deleteLater();
    d->reply     = nullptr;

    if (d->image)
    {
        delete d->image;
        d->image = nullptr;
    }

    if (d->workQueue.empty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received result without request";
        return;
    }

    // toInt() returns 0 if conversion fails. That fits nicely already.

    int netCode   = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto response = QJsonDocument::fromJson(reply->readAll());

    if ((netCode == 200) && !response.isEmpty())
    {
        /* Success! */

        IpfsTalkerResult result;
        result.action = &d->workQueue.first();

        switch (result.action->type)
        {
            case IpfsTalkerActionType::IMG_UPLOAD:
            {
                result.image.name = response.object()[QLatin1String("Name")].toString();
                result.image.size = response.object()[QLatin1String("Size")].toInt();
                result.image.url  = QLatin1String("https://ipfs.io/ipfs/") + response.object()[QLatin1String("Hash")].toString();
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Unexpected action";
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << response.toJson();
                break;
            }
        }

        Q_EMIT success(result);
    }
    else
    {
        if (netCode == 403)
        {
            /* HTTP 403 Forbidden -> Invalid token?
             * That needs to be handled internally, so don't Q_EMIT progress
             * and keep the action in the queue for later retries.
             */
            return;
        }
        else
        {
            /*
             * Failed.
             */
            auto msg = response.object()[QLatin1String("data")]
                       .toObject()[QLatin1String("error")]
                       .toString(QLatin1String("Could not read response."));

            Q_EMIT error(msg, d->workQueue.first());
        }
    }

    // Next work item.

    d->workQueue.dequeue();
    startWorkTimer();
}

void IpfsTalker::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != d->workTimer)
    {
        QObject::timerEvent(event);
        return;
    }

    event->accept();

    // One-shot only.

    QObject::killTimer(event->timerId());
    d->workTimer = 0;

    doWork();
}

void IpfsTalker::startWorkTimer()
{
    if (!d->workQueue.empty() && (d->workTimer == 0))
    {
        d->workTimer = QObject::startTimer(0);
        Q_EMIT busy(true);
    }
    else
    {
        Q_EMIT busy(false);
    }
}

void IpfsTalker::stopWorkTimer()
{
    if (d->workTimer != 0)
    {
        QObject::killTimer(d->workTimer);
        d->workTimer = 0;
    }
}

void IpfsTalker::doWork()
{
    if (d->workQueue.empty() || (d->reply != nullptr))
    {
        return;
    }

    auto &work = d->workQueue.first();

    switch (work.type)
    {
        case IpfsTalkerActionType::IMG_UPLOAD:
        {
            d->image = new QFile(work.upload.imgpath);

            if (!d->image->open(QIODevice::ReadOnly))
            {
                delete d->image;
                d->image = nullptr;

                /* Failed. */

                Q_EMIT error(i18n("Could not open file"), d->workQueue.first());

                d->workQueue.dequeue();
                doWork();
                return;
            }

            /* Set ownership to d->image to delete that as well. */

            auto* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, d->image);
            QHttpPart title;
            title.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QLatin1String("form-data; name=\"keyphrase\""));
            multipart->append(title);

            QHttpPart description;
            description.setHeader(QNetworkRequest::ContentDispositionHeader,
                                  QLatin1String("form-data; name=\"user\""));
            multipart->append(description);

            QHttpPart image;
            image.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QString::fromLatin1("form-data; name=\"file\";  filename=\"%1\"")
                            .arg(QLatin1String(QFileInfo(work.upload.imgpath).fileName().toUtf8().toPercentEncoding()))));
            image.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));
            image.setBodyDevice(d->image);
            multipart->append(image);
            QNetworkRequest request(QUrl(d->ipfsUploadUrl));
            d->reply = d->netMngr->post(request, multipart);

            break;
        }
    }

    if (d->reply)
    {
        connect(d->reply, &QNetworkReply::uploadProgress,
                this, &IpfsTalker::uploadProgress);

        connect(d->reply, &QNetworkReply::finished,
                this, &IpfsTalker::replyFinished);
    }
}

} // namespace DigikamGenericIpfsPlugin

#include "moc_ipfstalker.cpp"
