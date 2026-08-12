/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-17
 * Description : digital camera controller
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>
#include <QString>
#include <QFileInfo>

// Local includes

#include "downloadsettings.h"
#include "camiteminfo.h"
#include "dmetadata.h"
#include "dkcamera.h"
#include "dhistoryview.h"
#include "digikam_export.h"

namespace Digikam
{

class CameraCommand;

class DIGIKAM_GUI_EXPORT CameraController : public QThread
{
    Q_OBJECT

public:

    explicit CameraController(QWidget* const parent,
                              const QString& title,
                              const QString& model,
                              const QString& port,
                              const QString& path);
    ~CameraController()                                       override;

    bool cameraThumbnailSupport()                       const;
    bool cameraDeleteSupport()                          const;
    bool cameraUploadSupport()                          const;
    bool cameraMkDirSupport()                           const;
    bool cameraDelDirSupport()                          const;
    bool cameraCaptureImageSupport()                    const;
    bool cameraCaptureImagePreviewSupport()             const;

    QString cameraPath()                                const;
    QString cameraTitle()                               const;

    DKCamera::CameraDriverType cameraDriverType()       const;

    QByteArray cameraMD5ID()                            const;

    void capture();
    void listRootFolder(bool useMetadata);
    void listFolders(const QString& folder = QString());
    void listFiles(const QString& folder, bool useMetadata);
    void getFreeSpace();
    void getMetadata(const QString& folder, const QString& file);
    void getCameraInformation();
    void getPreview();

    /**
     * Get thumbnails for a list of camera items plus advanced information from metadata.
     */
    CameraCommand* getThumbsInfo(const CamItemInfoList& infoList, int thumbSize);
    void moveThumbsInfo(CameraCommand* const cmd);

    void download(const DownloadSettings& downloadSettings);
    void download(const DownloadSettingsList& list);
    void upload(const QFileInfo& srcFileInfo,
                const QString& destFile,
                const QString& destFolder);
    void deleteFile(const QString& folder, const QString& file);
    void lockFile(const QString& folder, const QString& file, bool lock);
    void openFile(const QString& folder, const QString& file);

    QIcon mimeTypeThumbnail(const QString& itemName)    const;

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLogMsg(const QString& msg,
                      DHistoryView::EntryType type,
                      const QString& folder,
                      const QString& file);
    void signalCameraInformation(const QString& summary,
                                 const QString& manual,
                                 const QString& about);
    void signalFreeSpace(qint64 bytesSize,
                         qint64 bytesAvail);
    void signalPreview(const QImage& preview);

    void signalConnected(bool val);
    void signalFolderList(const QStringList& folderList);
    void signalFileList(const CamItemInfoList& infoList);
    void signalUploaded(const CamItemInfo& itemInfo);
    void signalDownloaded(const QString& folder,
                          const QString& file,
                          const QString& temp,
                          int status);
    void signalDeleted(const QString& folder,
                       const QString& file,
                       bool status);
    void signalLocked(const QString& folder,
                      const QString& file,
                      bool status);
    void signalThumbInfo(const QString& folder,
                         const QString& file,
                         const CamItemInfo& itemInfo,
                         const QImage& thumb);
    void signalThumbInfoFailed(const QString& folder,
                               const QString& file,
                               const CamItemInfo& itemInfo);
    void signalMetadata(const QString& folder,
                        const QString& file,
                        const MetaEngineData& exifData);

    void signalInternalDownloadFailed(const QString& folder,
                                      const QString& file);
    void signalInternalUploadFailed(const QString& folder,
                                    const QString& file,
                                    const QString& src);
    void signalInternalDeleteFailed(const QString& folder,
                                    const QString& file);
    void signalInternalLockFailed(const QString& folder,
                                  const QString& file);

public Q_SLOTS:

    void slotCancel();
    void slotConnect();

protected:

    void run() override;
    void executeCommand(CameraCommand* const cmd);

private Q_SLOTS:

    void slotDownloadFailed(const QString& folder,
                            const QString& file);
    void slotUploadFailed(const QString& folder,
                          const QString& file,
                          const QString& src);
    void slotDeleteFailed(const QString& folder,
                          const QString& file);
    void slotLockFailed(const QString& folder,
                        const QString& file);

private:

    void sendLogMsg(const QString& msg,
                    DHistoryView::EntryType type = DHistoryView::StartingEntry,
                    const QString& folder = QString(),
                    const QString& file = QString());

    void addCommand(CameraCommand* const cmd);
    bool queueIsEmpty()                                 const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
