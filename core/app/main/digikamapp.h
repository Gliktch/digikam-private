/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText:      2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText:      2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAction>
#include <QString>
#include <QMenu>

// Local includes

#include "digikam_config.h"
#include "dxmlguiwindow.h"
#include "digikam_export.h"
#include "facetags.h"

namespace Digikam
{

class Album;
class ItemIconView;
class FaceScanSettings;
class FilterStatusBar;
class ItemInfo;
class ItemInfoList;
class CameraType;

class DIGIKAM_GUI_EXPORT DigikamApp : public DXmlGuiWindow
{
    Q_OBJECT

public:

    explicit DigikamApp();
    ~DigikamApp()                                                       override;

    virtual void show();
    void restoreSession();

    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);

    ItemIconView* view()    const;

    static DigikamApp* instance();

public:

    DInfoInterface* infoIface(DPluginAction* const ac)                  override;

Q_SIGNALS:

    void signalEscapePressed();
    void signalNextItem();
    void signalPrevItem();
    void signalFirstItem();
    void signalLastItem();
    void signalCutAlbumItemsSelection();
    void signalCopyAlbumItemsSelection();
    void signalPasteAlbumItemsSelection();

    void signalWindowHasMoved();
    void signalNotificationError(const QString& message, int type);

protected:

    bool queryClose()                                                   override;
    void moveEvent(QMoveEvent* e)                                       override;
    void closeEvent(QCloseEvent* e)                                     override;

private:

    void showThumbBar(bool visible)                                     override;
    void showSideBars(bool visible)                                     override;
    bool thumbbarVisibility() const                                     override;
    void customizedFullScreenMode(bool set)                             override;
    void customizedTrashView(bool set);
    void toggleShowBar();

private Q_SLOTS:

    void slotAlbumSelected(Album*);
    void slotImageSelected(const ItemInfoList&, const ItemInfoList&);
    void slotTrashSelectionChanged(const QString& text);
    void slotSelectionChanged(int selectionCount);
    void slotExit();
    void slotDBStat()                                                   override;
    void slotComponentsInfo()                                           override;
    void slotOnlineVersionCheck()                                       override;

    void slotRecurseAlbums(bool);
    void slotRecurseTags(bool);

    void slotAllGroupsOpen(bool);

    void slotAboutToShowForwardMenu();
    void slotAboutToShowBackwardMenu();

    void slotColorManagementOptionsChanged();
    void slotToggleColorManagedView();
    void slotSetCheckedExifOrientationAction(const ItemInfo& info);
    void slotResetExifOrientationActions();
    void slotTransformAction();

    void slotToggleLeftSideBar()                                        override;
    void slotToggleRightSideBar()                                       override;
    void slotPreviousLeftSideBarTab()                                   override;
    void slotNextLeftSideBarTab()                                       override;
    void slotPreviousRightSideBarTab()                                  override;
    void slotNextRightSideBarTab()                                      override;

    void slotToggleShowBar();

    void slotZoomSliderChanged(int);
    void slotThumbSizeChanged(int);
    void slotZoomChanged(double);

    void slotSwitchedToPreview();
    void slotSwitchedToIconView();
    void slotSwitchedToMapView();
    void slotSwitchedToTableView();
    void slotSwitchedToTrashView();

//@{
/// Internal setup methods implemented in digikamapp_setup.cpp

private:

    void setupView();
    void setupViewConnections();
    void setupStatusBar();
    void setupActions();
    void setupAccelerators();
    void setupExifOrientationActions();
    void setupImageTransformActions();
    void loadThemesActions();
    void loadCurrentTheme();
    void preloadWindows();
    void initGui();

//@}

//@{
/// Extra tool methods implemented in digikamapp_tools.cpp

private:

    void setupSelectToolsAction();

private Q_SLOTS:

    void slotMaintenance();
    void slotScanNewItems();
    void slotMaintenanceDone();
    void slotDatabaseMigration();
    void slotDetectFaces();
    void slotCheckFaceTrainingVersion();

//@}

//@{
/// Configure methods implemented in digikamapp_config.cpp

private:

    bool setup();
    bool setupICC();

private Q_SLOTS:

    void slotSetup()                                                    override;
    void slotSetupChanged();
    void slotEditKeys()                                                 override;
    void slotThemeChanged();

//@}

//@{
/// Import tools methods implemented in digikamapp_import.cpp

private:

    void updateQuickImportAction();

private Q_SLOTS:

    void slotImportedImagefromScanner(const QUrl& url);
    void slotImportAddImages();
    void slotImportAddFolders();

//@}

//@{
/// Camera management methods implemented in digikamapp_camera.cpp

public:

    void autoDetect();
    void downloadFrom(const QString& cameraGuiPath);
    void downloadFromUdi(const QString& udi);

Q_SIGNALS:

    void queuedOpenCameraUiFromPath(const QString& path);

private:

    void loadCameras();
    void updateCameraMenu();

private Q_SLOTS:

    void slotSetupCamera();
    void slotOpenManualCamera(QAction*);
    void slotCameraAdded(CameraType*);
    void slotCameraRemoved(QAction*);
    void slotCameraAutoDetect();
    void slotOpenCameraUiFromPath(const QString& path);
    void downloadImages(const QString& folder);
    void cameraAutoDetect();

//@}

//@{
/// Solid based methods implemented in digikamapp_solid.cpp

Q_SIGNALS:

    void queuedOpenSolidDevice(const QString& udi);

private:

    void fillSolidMenus();
    void connectToSolidNotifiers();
    void openSolidCamera(const QString& udi, const QString& label = QString());
    void openSolidUsmDevice(const QString& udi, const QString& label = QString());

private Q_SLOTS:

    void slotOpenSolidCamera(QAction*);
    void slotOpenSolidUsmDevice(QAction*);
    void slotOpenSolidDevice(const QString& udi);
    void slotSolidDeviceChanged(const QString& udi);

//@}

private:

    /// @note disabled
    DigikamApp(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;

    static DigikamApp* m_instance;
};

} // namespace Digikam
