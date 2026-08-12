/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QCloseEvent>
#include <QUrl>

// Local includes

#include "dxmlguiwindow.h"
#include "iteminfo.h"

namespace Digikam
{

class LightTableWindow : public DXmlGuiWindow
{
    Q_OBJECT

public:

    ~LightTableWindow()                                 override;

    static LightTableWindow* lightTableWindow();
    static bool              lightTableWindowCreated();

    void loadItemInfos(const ItemInfoList& list, const ItemInfo& imageInfoCurrent, bool addTo);
    void setLeftRightItems(const ItemInfoList& list, bool addTo);
    void refreshView();
    bool isEmpty()                                const;

    void toggleTag(int tagID);

public:

    DInfoInterface* infoIface(DPluginAction* const ac)  override;

Q_SIGNALS:

    void signalWindowHasMoved();

public Q_SLOTS:

    void slotFileChanged(const QString& filePath);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

protected:

    void moveEvent(QMoveEvent* e)                       override;

private:

    void customizedFullScreenMode(bool set)             override;
    void showSideBars(bool visible)                     override;
    void closeEvent(QCloseEvent* e)                     override;
    void showEvent(QShowEvent* e)                       override;

    void deleteItem(bool permanently);
    void deleteItem(const ItemInfo& info, bool permanently);

    /// @note disabled
    LightTableWindow();
    explicit LightTableWindow(QWidget*);

private Q_SLOTS:

    void slotBackward();
    void slotForward();
    void slotFirst();
    void slotLast();

    void slotSetItemLeft();
    void slotSetItemRight();
    void slotSetItemOnLeftPanel(const ItemInfo&);
    void slotSetItemOnRightPanel(const ItemInfo&);
    void slotLeftDroppedItems(const ItemInfoList&);
    void slotRightDroppedItems(const ItemInfoList&);

    void slotLeftPanelLeftButtonClicked();
    void slotRightPanelLeftButtonClicked();

    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);

    void slotLeftPreviewSelected(bool);
    void slotRightPreviewSelected(bool);

    void slotLeftZoomFactorChanged(double);
    void slotRightZoomFactorChanged(double);

    void slotToggleOnSyncPreview(bool);
    void slotToggleSyncPreview();
    void slotToggleNavigateByPair();

    void slotDeleteItem();
    void slotDeleteItem(const ItemInfo&);

    void slotDeleteFinalItem();
    void slotDeleteFinalItem(const ItemInfo&);

    void slotRemoveItem();
    void slotRemoveItem(const ItemInfo&);

    void slotItemSelected(const ItemInfo&);
    void slotClearItemsList();

    void slotThumbbarDroppedItems(const QList<ItemInfo>&);

    void slotToggleColorManagedView();
    void slotComponentsInfo()                           override;
    void slotDBStat()                                   override;
    void slotOnlineVersionCheck()                       override;

    void slotFileWithDefaultApplication();

    void slotRefreshStatusBar();

    void slotToggleLeftSideBar()                        override;
    void slotToggleRightSideBar()                       override;
    void slotPreviousLeftSideBarTab()                   override;
    void slotNextLeftSideBarTab()                       override;
    void slotPreviousRightSideBarTab()                  override;
    void slotNextRightSideBarTab()                      override;

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();
    void slotLeftSideBarActivateTitles();
    void slotLeftSideBarActivateComments();
    void slotLeftSideBarActivateAssignedTags();

// --- Internal setup methods implemented in lighttablewindow_config.cpp

public:

    void applySettings();

private:

    void readSettings();
    void writeSettings();

public Q_SLOTS:

    void slotSetup()                                    override;
    void slotColorManagementOptionsChanged();

    void slotThemeChanged();
    void slotApplicationSettingsChanged();

// --- Internal setup methods implemented in lighttablewindow_setup.cpp

private:

    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();

// --- Extra tool methods implemented in lighttablewindow_tools.cpp

private Q_SLOTS:

    void slotEditItem();
    void slotEditItem(const ItemInfo&);

    void slotLeftSlideShowManualFromCurrent();
    void slotRightSlideShowManualFromCurrent();
    void slotSlideShowLastItemUrl();

// --- Import tools methods implemented in lighttablewindow_import.cpp

private Q_SLOTS:

    void slotImportedImagefromScanner(const QUrl& url);

// --- Internal private container

private:

    class Private;
    Private* const d = nullptr;

    static LightTableWindow* m_instance;
};

} // namespace Digikam
