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

#pragma once

// Qt includes

#include <QObject>
#include <QNetworkReply>

// Local includes

#include "MarbleGlobal.h"
#include "digikam_export.h"

class QNetworkAccessManager;
class QString;
class QByteArray;
class QUrl;

namespace Marble
{
class HttpJobPrivate;

class DIGIKAM_EXPORT HttpJob: public QObject
{
    Q_OBJECT

public:

    HttpJob(const QUrl& sourceUrl, const QString& destFileName, const QString& id, QNetworkAccessManager* networkAccessManager);
    ~HttpJob() override;

    QUrl sourceUrl() const;
    void setSourceUrl(const QUrl&);

    QString initiatorId() const;
    void setInitiatorId(const QString&);

    QString destinationFileName() const;
    void setDestinationFileName(const QString&);

    bool tryAgain();

    DownloadUsage downloadUsage() const;
    void setDownloadUsage(const DownloadUsage);

    void setUserAgentPluginId(const QString& pluginId) const;

    QByteArray userAgent() const;

Q_SIGNALS:

    /**
     * errorCode contains 0, if there was no error and 1 otherwise
     */
    void jobDone(HttpJob*, int errorCode);
    void redirected(HttpJob* job, const QUrl& redirectionTarget);

    /**
     * This signal is emitted if the data was successfully received and
     * the argument data contains completely the downloaded content.
     */
    void dataReceived(HttpJob* job, const QByteArray& data);

public Q_SLOTS:

    void execute();

private Q_SLOTS:

    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();

private:

    Q_DISABLE_COPY(HttpJob)

private:

    HttpJobPrivate* const d = nullptr;

    friend class HttpJobPrivate;
};

} // namespace Marble
