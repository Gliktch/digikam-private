/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-05-27
 * Description : Implementation of v3 of the Imgur API
 *
 * SPDX-FileCopyrightText: 2016      by Fabian Vogt <fabian at ritter dash vogt dot de>
 * SPDX-FileCopyrightText: 2016-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imgurtalker.h"

// Qt includes

#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimerEvent>
#include <QUrlQuery>
#include <QFileInfo>
#include <QQueue>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dinfointerface.h"
#include "digikam_debug.h"
#include "wstoolutils.h"
#include "networkmanager.h"
#include "o0settingsstore.h"
#include "o0globals.h"


using namespace Digikam;

namespace DigikamGenericImgUrPlugin
{

class Q_DECL_HIDDEN ImgurTalker::Private
{
public:

    Private() = default;

public:

    /// API key and secret
    const QString             client_id             = QLatin1String("bd2572bce74b73d");
    const QString             client_secret         = QLatin1String("300988683e99cb7b203a5889cf71de9ac891c1c1");
    const QString             imgur_auth_url        = QLatin1String("https://api.imgur.com/oauth2/authorize");
    const QString             imgur_token_url       = QLatin1String("https://api.imgur.com/oauth2/token");
    const uint16_t            imgur_redirect_port   = 8000; // Redirect URI is http://127.0.0.1:8000      // krazy:exclude=insecurenet

    /// Handler for OAuth 2 related requests.
    O2                        auth;

    /// Work queue.
    QQueue<ImgurTalkerAction> workQueue;

    /// ID of timer triggering on idle (0ms).
    int                       workTimer             = 0;

    /// Current QNetworkReply instance.
    QNetworkReply*            reply                 = nullptr;

    /// Current image being uploaded.
    QFile*                    image                 = nullptr;

    /// The QNetworkAccessManager instance used for connections.
    QNetworkAccessManager*    netMngr               = nullptr;
};

ImgurTalker::ImgurTalker(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);

    d->auth.setClientId(d->client_id);
    d->auth.setClientSecret(d->client_secret);
    d->auth.setRequestUrl(d->imgur_auth_url);
    d->auth.setTokenUrl(d->imgur_token_url);
    d->auth.setRefreshTokenUrl(d->imgur_token_url);
    d->auth.setLocalPort(d->imgur_redirect_port);
    d->auth.setLocalhostPolicy(QString());

    QSettings* const settings    = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(settings,
                                                       QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Imgur"));
    d->auth.setStore(store);

    connect(&d->auth, &O2::linkedChanged,
            this, &ImgurTalker::slotOauthAuthorized);

    connect(&d->auth, &O2::openBrowser,
            this, &ImgurTalker::slotOauthRequestPin);

    connect(&d->auth, &O2::linkingFailed,
            this, &ImgurTalker::slotOauthFailed);
}

ImgurTalker::~ImgurTalker()
{
    // Disconnect all signals as cancelAllWork may emit.

    disconnect(this, nullptr, nullptr, nullptr);
    cancelAllWork();

    delete d;
}

O2& ImgurTalker::getAuth()
{
    return d->auth;
}

unsigned int ImgurTalker::workQueueLength()
{
    return d->workQueue.size();
}

void ImgurTalker::queueWork(const ImgurTalkerAction& action)
{
    d->workQueue.enqueue(action);
    startWorkTimer();
}

void ImgurTalker::cancelAllWork()
{
    stopWorkTimer();

    if (d->reply)
    {
        d->reply->abort();
    }

    // Should signalError be emitted for those actions?

    while (!d->workQueue.isEmpty())
    {
        d->workQueue.dequeue();
    }
}

QUrl ImgurTalker::urlForDeletehash(const QString& deletehash)
{
    return QUrl{QLatin1String("https://imgur.com/delete/") + deletehash};
}

void ImgurTalker::slotOauthAuthorized()
{
    bool success = d->auth.linked();

    if (success)
    {
        startWorkTimer();
    }
    else
    {
        Q_EMIT signalBusy(false);
    }

    Q_EMIT signalAuthorized(success,
                            d->auth.extraTokens().value(QLatin1String("account_username")).toString());
}

void ImgurTalker::slotOauthRequestPin(const QUrl& url)
{
    Q_EMIT signalBusy(false);
    Q_EMIT signalRequestPin(url);
}

void ImgurTalker::slotOauthFailed()
{
    cancelAllWork();

    Q_EMIT signalAuthError(i18n("Could not authorize"));
}

void ImgurTalker::slotUploadProgress(qint64 sent, qint64 total)
{
    // Don't divide by 0

    if (total > 0)
    {
        Q_EMIT signalProgress((sent * 100) / total, d->workQueue.first());
    }
}

void ImgurTalker::slotReplyFinished()
{
    auto* const reply = d->reply;
    reply->deleteLater();
    d->reply          = nullptr;

    if (d->image)
    {
        delete d->image;
        d->image = nullptr;
    }

    if (d->workQueue.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received result without request";
        return;
    }

    // NOTE: toInt() returns 0 if conversion fails. That fits nicely already.

    int netcode   = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto response = QJsonDocument::fromJson(reply->readAll());

    if ((netcode == 200) && !response.isEmpty())
    {
        // Success!

        ImgurTalkerResult result;
        result.action   = &d->workQueue.first();
        const auto data = response.object().value(QLatin1String("data")).toObject();

        switch (result.action->type)
        {
            case ImgurTalkerActionType::IMG_UPLOAD:
            case ImgurTalkerActionType::ANON_IMG_UPLOAD:
            {
                result.image.animated    = data[QLatin1String("animated")].toBool();
                result.image.bandwidth   = data[QLatin1String("bandwidth")].toInt();
                result.image.datetime    = data[QLatin1String("datetime")].toInt();
                result.image.deletehash  = data[QLatin1String("deletehash")].toString();
                result.image.description = data[QLatin1String("description")].toString();
                result.image.height      = data[QLatin1String("height")].toInt();
                result.image.hash        = data[QLatin1String("id")].toString();
                result.image.name        = data[QLatin1String("name")].toString();
                result.image.size        = data[QLatin1String("size")].toInt();
                result.image.title       = data[QLatin1String("title")].toString();
                result.image.type        = data[QLatin1String("type")].toString();
                result.image.url         = data[QLatin1String("link")].toString();
                result.image.views       = data[QLatin1String("views")].toInt();
                result.image.width       = data[QLatin1String("width")].toInt();
                break;
            }

            case ImgurTalkerActionType::ACCT_INFO:
            {
                result.account.username = data[QLatin1String("url")].toString();

                // TODO: Other fields.

                break;
            }

            default:
            {
                qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Unexpected action";
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << response.toJson();
                break;
            }
        }

        Q_EMIT signalSuccess(result);
    }
    else
    {
        if (netcode == 403)
        {
            /**
             * HTTP 403 Forbidden -> Invalid token?
             * That needs to be handled internally, so don't Q_EMIT signalProgress
             * and keep the action in the queue for later retries.
             */
            d->auth.refresh();

            return;
        }
        else
        {
            // Failed.

            const auto msg = response.object().value(QLatin1String("data"))
                                              .toObject().value(QLatin1String("error"))
                                              .toString(QLatin1String("Could not read response."));

            Q_EMIT signalError(msg, d->workQueue.first());
        }
    }

    // Next work item.

    d->workQueue.dequeue();
    startWorkTimer();
}

void ImgurTalker::timerEvent(QTimerEvent* event)
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

void ImgurTalker::startWorkTimer()
{
    if (!d->workQueue.isEmpty() && (d->workTimer == 0))
    {
        d->workTimer = QObject::startTimer(0);

        Q_EMIT signalBusy(true);
    }
    else
    {
        Q_EMIT signalBusy(false);
    }
}

void ImgurTalker::stopWorkTimer()
{
    if (d->workTimer != 0)
    {
        QObject::killTimer(d->workTimer);
        d->workTimer = 0;
    }
}

void ImgurTalker::addAuthToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Bearer %1").arg(d->auth.token()).toUtf8());
}

void ImgurTalker::addAnonToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Client-ID %1").arg(d->auth.clientId()).toUtf8());
}

void ImgurTalker::doWork()
{
    if (d->workQueue.isEmpty() || (d->reply != nullptr))
    {
        return;
    }

    auto &work = d->workQueue.first();

    if ((work.type != ImgurTalkerActionType::ANON_IMG_UPLOAD) && !d->auth.linked())
    {
        d->auth.link();

        return; // Wait for the signalAuthorized() signal.
    }

    switch (work.type)
    {
        case ImgurTalkerActionType::ACCT_INFO:
        {
            QNetworkRequest request(QUrl(QString::fromLatin1("https://api.imgur.com/3/account/%1")
                                        .arg(QLatin1String(work.account.username.toUtf8().toPercentEncoding()))));
            addAuthToken(&request);

            d->reply = d->netMngr->get(request);
            break;
        }

        case ImgurTalkerActionType::ANON_IMG_UPLOAD:
        case ImgurTalkerActionType::IMG_UPLOAD:
        {
            d->image = new QFile(work.upload.imgpath);

            if (!d->image->open(QIODevice::ReadOnly))
            {
                delete d->image;
                d->image = nullptr;

                // Failed.

                Q_EMIT signalError(i18n("Could not open file"), d->workQueue.first());

                d->workQueue.dequeue();
                doWork();
                return;
            }

            // Set ownership to d->image to delete that as well.

            auto* const multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, d->image);
            QHttpPart title;
            title.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QLatin1String("form-data; name=\"title\""));
            title.setBody(work.upload.title.toUtf8().toPercentEncoding());
            multiPart->append(title);

            QHttpPart description;
            description.setHeader(QNetworkRequest::ContentDispositionHeader,
                                  QLatin1String("form-data; name=\"description\""));
            description.setBody(work.upload.description.toUtf8().toPercentEncoding());
            multiPart->append(description);

            QHttpPart imagePart;
            imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QString::fromLatin1("form-data; name=\"image\"; filename=\"%1\"")
                            .arg(QLatin1String(QFileInfo(work.upload.imgpath).fileName().toUtf8().toPercentEncoding()))));
            imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
            imagePart.setBodyDevice(d->image);
            multiPart->append(imagePart);

            QNetworkRequest request(QUrl(QLatin1String("https://api.imgur.com/3/image")));

            if (work.type == ImgurTalkerActionType::IMG_UPLOAD)
            {
                addAuthToken(&request);
            }
            else
            {
                addAnonToken(&request);
            }

            d->reply = d->netMngr->post(request, multiPart);

            // delete the multiPart with the reply

            multiPart->setParent(d->reply);

            break;
        }
    }

    if (d->reply)
    {
        connect(d->reply, &QNetworkReply::uploadProgress,
                this, &ImgurTalker::slotUploadProgress);

        connect(d->reply, &QNetworkReply::finished,
                this, &ImgurTalker::slotReplyFinished);
    }
}

} // namespace DigikamGenericImgUrPlugin

#include "moc_imgurtalker.cpp"
