/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QString>
#include <QImage>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMultiMap>
#include <QUrl>

// Local includes

#include "camiteminfo.h"
#include "dhistoryview.h"
#include "dmetadata.h"
#include "downloadsettings.h"
#include "importiconview.h"
#include "dxmlguiwindow.h"
#include "digikam_export.h"

/*
#include "camerahistoryupdater.h"
*/

namespace Digikam
{

class Album;
class PAlbum;
class CollectionLocation;
class CameraHistoryUpdater;
class ImportIconView;
class CameraThumbsCtrl;

class DIGIKAM_GUI_EXPORT ImportUI : public DXmlGuiWindow
{
    Q_OBJECT

public:

    explicit ImportUI(const QString& cameraTitle,
                      const QString& model,
                      const QString& port,
                      const QString& path,
                      int startIndex);
    ~ImportUI()                                           override;

    static ImportUI* instance();

    bool isBusy()                                   const;
    bool isClosed()                                 const;

    bool    cameraDeleteSupport()                   const;
    bool    cameraUploadSupport()                   const;
    bool    cameraMkDirSupport()                    const;
    bool    cameraDelDirSupport()                   const;
    bool    cameraUseUMSDriver()                    const;
    bool    cameraUseGPhotoDriver()                 const;
    QString cameraTitle()                           const;

    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);

    DownloadSettings downloadSettings()             const;

    CameraThumbsCtrl* getCameraThumbsCtrl()         const;

public:

    DInfoInterface* infoIface(DPluginAction* const)       override
    {
        return nullptr;
    };

Q_SIGNALS:

    void signalLastDestination(const QUrl&);
    void signalWindowHasMoved();
    void signalEscapePressed();
    void signalPreviewRequested(const CamItemInfo&, bool);
    void signalNewSelection(bool);

public Q_SLOTS:

    void slotDownload(bool onlySelected, bool deleteAfter, Album* pAlbum = nullptr);
    void slotUploadItems(const QList<QUrl>&); // public to be used in drag'n'drop

protected:

    void closeEvent(QCloseEvent* e)                       override;
    void moveEvent(QMoveEvent* e)                         override;

private:

    void setupActions();
    void updateActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void setupAccelerators();
    void setupCameraController(const QString& model, const QString& port, const QString& path);

    void readSettings();
    void saveSettings();
    bool createAutoAlbum(const QUrl& parentURL, const QString& sub,
                         const QDate& date, QString& errMsg) const;

    bool dialogClosed();
    void finishDialog();
    void refreshFreeSpace();
    void refreshCollectionFreeSpace();
    void deleteItems(bool onlySelected, bool onlyDownloaded);
    void toggleLock(CamItemInfo& info);
    void setDownloaded(CamItemInfo& itemInfo, int status);
    void itemsSelectionSizeInfo(qint64& fSizeBytes, qint64& dSizeBytes);
    QMap<QString, int> countItemsByFolders() const;
    void checkItem4Deletion(const CamItemInfo& info, QStringList& folders, QStringList& files,
                            CamItemInfoList& deleteList, CamItemInfoList& lockedList);

    QString identifyCategoryforMime(const QString& mime);

    bool checkDiskSpace(PAlbum* pAlbum);
    bool downloadCameraItems(PAlbum* pAlbum, bool onlySelected, bool deleteAfter);
    bool createSubAlbums(QUrl& downloadUrl, const QString& suffix, const QDateTime& dateTime);
    bool createSubAlbum(QUrl& downloadUrl, const QString& subalbum, const QDate& date);
    bool createDateBasedSubAlbum(QUrl& downloadUrl, const QDateTime& dateTime);
    bool createExtBasedSubAlbum(QUrl& downloadUrl, const QString& suffix, const QDate& date);
    void postProcessAfterDownload();


    void showThumbBar(bool visible)                       override;
    void showSideBars(bool visible)                       override;
    bool thumbbarVisibility() const                       override;
    void customizedFullScreenMode(bool set)               override;
    void toogleShowBar();
    void setInitialSorting();
    void sidebarTabTitleStyleChanged();
    void updateRightSideBar(const CamItemInfo& info);

private Q_SLOTS:

    void slotCancelButton();

    void slotShowLog();
    void slotConnected(bool val);
    void slotBusy(bool val);
    void slotLogMsg(const QString& msg, DHistoryView::EntryType type,
                    const QString& folder = QString(), const QString& file = QString());
    void slotInformation();
    void slotCapture();
    void slotCameraInformation(const QString&, const QString&, const QString&);
    void slotCameraFreeSpaceInfo(qint64 bytesSize, qint64 bytesAvail);
    void slotCollectionLocationStatusChanged(const CollectionLocation& location, int oldStatus);
    void slotHistoryEntryClicked(const QVariant&);

    void slotFolderList(const QStringList& folderList);

    void slotZoomSliderChanged(int size);
    void slotZoomChanged(double zoom);
    void slotThumbSizeChanged(int size);

    void slotUpload();

    void slotDownloadNew();
    void slotDownloadSelected();
    void slotDownloadAll();

    void slotDownloadAndDeleteNew();
    void slotDownloadAndDeleteSelected();
    void slotDownloadAndDeleteAll();

    void slotDeleteAfterDownload();
    void slotDeleteSelected();
    void slotDeleteNew();
    void slotDeleteAll();

    void slotToggleLock();
    void slotMarkAsDownloaded();

    void slotUploaded(const CamItemInfo&);
    void slotDownloaded(const QString&, const QString&, const QString&, int);
    void slotDeleted(const QString&, const QString&, bool);
    void slotLocked(const QString&, const QString&, bool);

    void slotUpdateRenamePreview();
    void slotSelectNew();
    void slotSelectLocked();
    void slotProgressTimerDone();

    void slotNewSelection(bool);
    void slotImageSelected(const CamItemInfoList& selection, const CamItemInfoList& listAll);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();

    void slotMetadata(const QString& folder, const QString& file, const MetaEngineData& meta);

    void setFilter(Filter*);

    void slotToggleShowBar();
    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotSetup()                                      override;
    void slotComponentsInfo()                             override;
    void slotDBStat()                                     override;
    void slotOnlineVersionCheck()                         override;
    void slotToggleRightSideBar()                         override;
    void slotPreviousRightSideBarTab()                    override;
    void slotNextRightSideBarTab()                        override;

    void slotSetupChanged();

private:

    /// @note disabled
    ImportUI(QWidget*) = delete;

private:

    class Private;
    Private* const   d = nullptr;

    static ImportUI* m_instance;
};

} // namespace Digikam
