/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam Survey tool GUI
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

class DigikamItemView;
class StackedView;

class SurveyWindow : public DXmlGuiWindow
{
    Q_OBJECT

public:

    ~SurveyWindow()                                     override;

    static SurveyWindow* surveyWindow();
    static bool          surveyWindowCreated();

    void init(DigikamItemView* const iconView);

    void refreshView();
    bool isSleeping()                             const;
    bool isEmpty()                                const;

    void toggleTag(int tagID);

public:

    DInfoInterface* infoIface(DPluginAction* const ac)  override;

Q_SIGNALS:

    void signalWindowHasMoved();
    void signalAssignPickLabel(int pickId);
    void signalAssignColorLabel(int colorId);
    void signalAssignRating(int rating, bool toggle);

public Q_SLOTS:

    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating, bool toggle);

protected:

    void moveEvent(QMoveEvent* e)                       override;

private:

    void customizedFullScreenMode(bool set)             override;
    void showSideBars(bool visible)                     override;
    void closeEvent(QCloseEvent* e)                     override;
    void showEvent(QShowEvent* e)                       override;
    void changeEvent(QEvent* e)                         override;

private:

    /// @note disabled
    SurveyWindow();
    explicit SurveyWindow(QWidget*);

private Q_SLOTS:

    void slotBackward();
    void slotForward();
    void slotFirst();
    void slotLast();

    void slotZoomFactorChanged(double);

    void slotDeleteItem();
    void slotDeleteFinalItem();

    void slotItemSelected();

    void slotToggleColorManagedView();
    void slotComponentsInfo()                           override;
    void slotDBStat()                                   override;
    void slotOnlineVersionCheck()                       override;

    void slotFileWithDefaultApplication();

    void slotToggleRightSideBar()                       override;
    void slotPreviousRightSideBarTab()                  override;
    void slotNextRightSideBarTab()                      override;

    void slotSideBarActivateTitles();
    void slotSideBarActivateComments();
    void slotSideBarActivateAssignedTags();

// --- Internal setup methods implemented in surveywindow_config.cpp

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

// --- Internal setup methods implemented in surveywindow_setup.cpp

private:

    void setupActions();
    void setupConnections();
    void setupUserArea(DigikamItemView* const iconView);
    void setupStatusBar();

// --- Extra tool methods implemented in surveywindow_tools.cpp

private Q_SLOTS:

    void slotEditItem();
    void slotEditItem(const ItemInfo&);

    void slotSlideShowManualFromCurrent();
    void slotSlideShowLastItemUrl();

// --- Internal private container

private:

    class Private;
    Private* const d = nullptr;

    static SurveyWindow* m_instance;
};

} // namespace Digikam
