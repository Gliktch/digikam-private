/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "HttpJob.h"

// Qt includes

#include <QNetworkAccessManager>

// Local includes

#include "HttpDownloadManager.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN HttpJobPrivate
{
public:

    HttpJobPrivate(const QUrl& sourceUrl, const QString& destFileName,
                   const QString& id, QNetworkAccessManager* networkAccessManager);

public:

    QUrl                            m_sourceUrl;
    QString                         m_destinationFileName;
    QString                         m_initiatorId;
    int                             m_trialsLeft;
    DownloadUsage                   m_downloadUsage;
    QString                         m_userAgent;
    QNetworkAccessManager* const    m_networkAccessManager  = nullptr;
    QNetworkReply*                  m_networkReply          = nullptr;
};

HttpJobPrivate::HttpJobPrivate(const QUrl& sourceUrl, const QString& destFileName,
                               const QString& id, QNetworkAccessManager* networkAccessManager)
    : m_sourceUrl(sourceUrl),
      m_destinationFileName(destFileName),
      m_initiatorId(id),
      m_trialsLeft(3),
      m_downloadUsage(DownloadBrowse),

      // FIXME: remove initialization depending on if empty pluginId
      // results in valid user agent string

      m_userAgent(QString::fromUtf8("unknown")),
      m_networkAccessManager(networkAccessManager),
      m_networkReply(nullptr)
{
}


HttpJob::HttpJob(const QUrl& sourceUrl, const QString& destFileName,
                 const QString& id, QNetworkAccessManager* networkAccessManager)
    : d(new HttpJobPrivate(sourceUrl, destFileName, id, networkAccessManager))
{
}

HttpJob::~HttpJob()
{
    delete d;
}

QUrl HttpJob::sourceUrl() const
{
    return d->m_sourceUrl;
}

void HttpJob::setSourceUrl(const QUrl& url)
{
    d->m_sourceUrl = url;
}

QString HttpJob::initiatorId() const
{
    return d->m_initiatorId;
}

void HttpJob::setInitiatorId(const QString& id)
{
    d->m_initiatorId = id;
}

QString HttpJob::destinationFileName() const
{
    return d->m_destinationFileName;
}

void HttpJob::setDestinationFileName(const QString& fileName)
{
    d->m_destinationFileName = fileName;
}

bool HttpJob::tryAgain()
{
    if (d->m_trialsLeft > 0)
    {
        d->m_trialsLeft--;
        return true;
    }
    else
    {
        return false;
    }
}

DownloadUsage HttpJob::downloadUsage() const
{
    return d->m_downloadUsage;
}

void HttpJob::setDownloadUsage(const DownloadUsage usage)
{
    d->m_downloadUsage = usage;
}

void HttpJob::setUserAgentPluginId(const QString& pluginId) const
{
    d->m_userAgent = pluginId;
}

QByteArray HttpJob::userAgent() const
{
    switch (d->m_downloadUsage)
    {
        case DownloadBrowse:
            return HttpDownloadManager::userAgent(QString::fromUtf8("Browser"), d->m_userAgent);

        case DownloadBulk:
            return HttpDownloadManager::userAgent(QString::fromUtf8("BulkDownloader"), d->m_userAgent);

        default:
            qCCritical(DIGIKAM_GEOENGINE_LOG) << "Unknown download usage value:" << d->m_downloadUsage;

            return HttpDownloadManager::userAgent(QString::fromUtf8("unknown"), d->m_userAgent);
    }
}

void HttpJob::execute()
{
    QNetworkRequest request(d->m_sourceUrl);
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setRawHeader(QByteArray("User-Agent"),
                         userAgent());
    d->m_networkReply = d->m_networkAccessManager->get(request);

    connect(d->m_networkReply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)));

    connect(d->m_networkReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)),
            SLOT(error(QNetworkReply::NetworkError)));

    connect(d->m_networkReply, SIGNAL(finished()),
            SLOT(finished()));
}
void HttpJob::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);

    //     qCDebug(DIGIKAM_GEOENGINE_LOG) << "downloadProgress" << destinationFileName()
    //              << bytesReceived << '/' << bytesTotal;
}

void HttpJob::error(QNetworkReply::NetworkError code)
{
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "error" << destinationFileName() << code;
}

void HttpJob::finished()
{
    QNetworkReply::NetworkError const error = d->m_networkReply->error();

    //     qCDebug(DIGIKAM_GEOENGINE_LOG) << "finished" << destinationFileName()
    //              << "error" << error;

    const QVariant httpPipeliningWasUsed = d->m_networkReply->attribute(QNetworkRequest::HttpPipeliningWasUsedAttribute);

    if (!httpPipeliningWasUsed.isNull())
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "http pipelining used:" << httpPipeliningWasUsed.toBool();
    }

    switch (error)
    {
        case QNetworkReply::NoError:
        {
            // check if we are redirected

            const QVariant redirectionAttribute = d->m_networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

            if (!redirectionAttribute.isNull())
            {
                Q_EMIT redirected(this, redirectionAttribute.toUrl());
            }

            else
            {
                // no redirection occurred

                const QByteArray data = d->m_networkReply->readAll();

                Q_EMIT dataReceived(this, data);
            }

            break;
        }

        default:
        {
            Q_EMIT jobDone(this, 1);

            break;
        }
    }

    d->m_networkReply->disconnect(this);

    // No delete. This method is called by a signal QNetworkReply::finished.

    d->m_networkReply->deleteLater();
    d->m_networkReply = nullptr;
}

} // namespace Marble

#include "moc_HttpJob.cpp"
