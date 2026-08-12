/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-14
 * Description : Files downloader
 *
 * SPDX-FileCopyrightText: 2020-2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>
#include <QSettings>

// Local includes

#include "digikam_export.h"
#include "downloadinfo.h"

class QNetworkReply;

namespace Digikam
{

class DIGIKAM_EXPORT FilesDownloader : public QDialog
{
    Q_OBJECT

public:

    explicit FilesDownloader(QWidget* const parent = nullptr);
    ~FilesDownloader()                                  override;

    bool checkDownloadFiles() const;
    void startDownload();

    static QString getFacesEnginePath();

private:

    void download();
    void nextDownload();
    void createRequest(const QUrl& url);
    void printDownloadInfo(const QUrl& url);
    bool downloadExists(const DownloadInfo& info) const;
    void deleteUnusedFiles()                      const;

    void createDownloadInfo();

private Q_SLOTS:

    void reject()                                       override;
    void slotDownload();
    void slotHelp();
    void slotUpdateDownloadInfo();
    void slotDownloaded(QNetworkReply* reply);
    void slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void slotFacesEnginePathChanged();
    void slotOpenLocalRepo();

private:

    /// @note disabled
    FilesDownloader(const FilesDownloader&)            = delete;
    FilesDownloader& operator=(const FilesDownloader&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
