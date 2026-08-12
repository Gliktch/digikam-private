/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-31
 * Description : Online version downloader.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QNetworkReply>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT OnlineVersionDwnl : public QObject
{
    Q_OBJECT

public:

    explicit OnlineVersionDwnl(QObject* const parent = nullptr,
                               bool checkPreRelease = false,
                               bool updateWithDebug = false);
    ~OnlineVersionDwnl() override;

    void startDownload(const QString& version);
    void cancelDownload();

    QString downloadedPath() const;

    QString downloadUrl()    const;

Q_SIGNALS:

    void signalDownloadError(const QString& error);
    void signalDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void signalComputeChecksum();

private Q_SLOTS:

    void slotDownloaded(QNetworkReply* reply);

private:

    void download(const QUrl& url);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
