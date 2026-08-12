/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2001-2003 by Joseph Wenninger <jowenn at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QSplitter>
#include <QPushButton>
#include <QWidget>
#include <QList>
#include <QStyleOptionToolButton>
#include <QBoxLayout>

// Local includes

#include "digikam_export.h"
#include "statesavingobject.h"

class KConfigGroup;

namespace Digikam
{

class DMultiTabBarButton;
class DMultiTabBarTab;

/**
 * @brief a Widget for horizontal and vertical tabs.
 */
class DIGIKAM_EXPORT DMultiTabBar : public QWidget
{
    Q_OBJECT

public:

    /**
     * @brief The list of available styles for DMultiTabBar
     */
    enum TextStyle
    {
        ActiveIconText = 0, ///< Always shows icon, only show the text of active tabs.
        AllIconsText   = 2  ///< Always shows the text and icons.
    };

public:

    explicit DMultiTabBar(Qt::Edge pos, QWidget* const parent = nullptr);
    ~DMultiTabBar() override;

    /**
     * @brief append  a new button to the button area. The button can later on be accessed with button(ID)
     * eg for connecting signals to it
     * @param pic a icon for the button
     * @param id an arbitrary ID value. It will be emitted in the clicked signal for identifying the button
     *  if more than one button is connected to a signals.
     * @param popup A popup menu which should be displayed if the button is clicked
     * @param not_used_yet will be used for a popup text in the future
     */
    void appendButton(const QIcon& pic,
                      int id = -1,
                      QMenu* const popup = nullptr,
                      const QString& not_used_yet = QString());

    /**
     * @brief remove a button with the given ID
     */
    void removeButton(int id);

    /**
     * @brief append a new tab to the tab area. It can be accessed later on with tabb(id);
     * @param pic a icon for the tab
     * @param id an arbitrary ID which can be used later on to identify the tab
     * @param text if a mode with text is used it will be the tab text, otherwise a mouse over hint
     */
    void appendTab(const QIcon& pic,
                   int id = -1,
                   const QString& text = QString());

    /**
     * @brief remove a tab with a given ID
     */
    void removeTab(int id);

    /**
     * @brief set a tab to "raised"
     * @param id The ID of the tab to manipulate
     * @param state true == activated/raised, false == not active
     */
    void setTab(int id, bool state);

    /**
     * @return the state of a tab, identified by its ID
     */
    bool isTabRaised(int id)           const;

    /**
     * @brief get a pointer to a button within the button area identified by its ID
     */
    DMultiTabBarButton* button(int id) const;

    /**
     * @brief get a pointer to a tab within the tab area, identified by its ID
     */
    DMultiTabBarTab* tab(int id)       const;

    /**
     * @brief set the real position of the widget.
     * @param pos if the mode is horizontal, only use top, bottom, if it is vertical use left or right
     */
    void setPosition(Qt::Edge pos);

    /**
     * @brief get the tabbar position.
     * @return position
     */
    Qt::Edge position()                const;

    /**
     * @brief set the display style of the tabs
     */
    void setStyle(TextStyle style);

    /**
     * @brief get the display style of the tabs
     * @return display style
     */
    TextStyle tabStyle()               const;

protected:

    void updateSeparator();

    virtual void fontChange(const QFont&);

private:

    friend class DMultiTabBarButton;

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------------------

class DIGIKAM_EXPORT DMultiTabBarButton: public QPushButton
{
    Q_OBJECT

public:

    ~DMultiTabBarButton()         override;

    int id() const;

public Q_SLOTS:

    void setText(const QString& text);

Q_SIGNALS:

    /**
     * @brief this is emitted if the button is clicked
     * @param id the ID identifying the button
     */
    void signalClicked(int id);

protected Q_SLOTS:

    virtual void slotClicked();

protected:

    DMultiTabBarButton(const QIcon& pic,
                       const QString&,
                       int id,
                       QWidget* const parent);

    void hideEvent(QHideEvent*)   override;
    void showEvent(QShowEvent*)   override;
    void paintEvent(QPaintEvent*) override;

private:

    friend class DMultiTabBar;

    int m_id;
};

// -------------------------------------------------------------------------------------

class DIGIKAM_EXPORT DMultiTabBarTab: public DMultiTabBarButton
{
    Q_OBJECT

public:

    ~DMultiTabBarTab()            override;

    QSize sizeHint()        const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:

    /**
     * @brief this is used internally, but can be used by the user.
     * It the according call of DMultiTabBar is invoked though this modifications will be overwritten
     */
    void setPosition(Qt::Edge);

    /**
     * @brief this is used internally, but can be used by the user.
     * It the according call of DMultiTabBar is invoked though this modifications will be overwritten
     */
    void setStyle(DMultiTabBar::TextStyle);

    /**
     * @brief set the active state of the tab
     * @param  state true==active false==not active
     */
    void setState(bool state);

    void setIcon(const QString&);
    void setIcon(const QIcon&);

protected:

    void    computeMargins (int* hMargin, int* vMargin)        const;
    QSize   computeSizeHint(bool withText)                     const;
    bool    shouldDrawText()                                   const;
    bool    isVertical()                                       const;
    QPixmap iconPixmap()                                       const;
    void    initButtonStyleOption(QStyleOptionToolButton* opt) const;

    friend class DMultiTabBarFrame;

    /**
     * @brief This class should never be created except with the appendTab call of DMultiTabBar
     */
    DMultiTabBarTab(const QIcon& pic,
                    const QString&,
                    int id,
                    QWidget* const parent,
                    Qt::Edge pos,
                    DMultiTabBar::TextStyle style);

    void paintEvent(QPaintEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------------------

class DMultiTabBarFrame: public QFrame
{
    Q_OBJECT

public:

    explicit DMultiTabBarFrame(QWidget* const parent, Qt::Edge pos);
    ~DMultiTabBarFrame()                override;

    void appendTab(const QIcon&, int = -1, const QString& = QString());
    DMultiTabBarTab* tab(int) const;
    void removeTab(int);
    void setPosition(Qt::Edge pos);
    void setStyle(DMultiTabBar::TextStyle style);
    void showActiveTabTexts(bool show);
    QList<DMultiTabBarTab*>* tabs();

protected:

    /**
     * @brief Reimplemented from QScrollView
     * in order to ignore all mouseEvents on the viewport, so that the
     * parent can handle them.
     */
    virtual void contentsMousePressEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*)  override;

private:

    friend class DMultiTabBar;

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------------------

class SidebarSplitter;

/**
 * @brief This class handles a sidebar view
 *
 * Since this class derives from StateSavingObject, you can call
 * StateSavingObject#loadState() and StateSavingObject#saveState()
 * for loading/saving of settings. However, if you use multiple
 * sidebar instances in your program, you have to remember to either
 * call QObject#setObjectName(), StateSavingObject#setEntryPrefix() or
 * StateSavingObject#setConfigGroup() first.
 */
class DIGIKAM_EXPORT Sidebar : public DMultiTabBar,
                               public StateSavingObject
{
    Q_OBJECT

public:

    /**
     * @brief Creates a new sidebar
     * @param parent sidebar's parent
     * @param sp sets the splitter, which should handle the width. The splitter normally
     *           is part of the main view. Internally, the width of the widget stack can
     *           be changed by a QSplitter.
     * @param side where the sidebar should be displayed. At the left or right border.
     *             Use Qt::LeftEdge or Qt::RightEdge.
     * @param minimizedDefault hide the sidebar when the program is started the first time.
     */
    explicit Sidebar(QWidget* const parent,
                     SidebarSplitter* const sp,
                     Qt::Edge side = Qt::LeftEdge,
                     bool minimizedDefault = false);

    ~Sidebar()                              override;

    SidebarSplitter* splitter()       const;

    /**
     * @brief Appends a new tab to the sidebar
     * @param w widget which is activated by this tab
     * @param pic icon which is shown in this tab
     * @param title text which is shown it this tab
     */
    void appendTab(QWidget* const w, const QIcon& pic, const QString& title);

    /**
     * @brief Deletes a tab from the tabbar
     */
    void deleteTab(QWidget* const w);

    /**
     * @brief Activates a tab
     */
    void setActiveTab(QWidget* const w);

    /**
     * @brief Activates a next tab from current one. If current one is last, first one is activated.
     */
    void activeNextTab();

    /**
     * @brief Activates a previous tab from current one. If current one is first, last one is activated.
     */
    void activePreviousTab();

    /**
     * @return the currently activated tab, or 0 if no tab is active
     */
    QWidget* getActiveTab()           const;

    /**
     * @brief Hides the sidebar (display only the activation buttons)
     */
    void shrink();

    /**
     * @brief Redisplays the whole sidebar
     */
    void expand();

    /**
     * @brief Hide sidebar and backup minimized state.
     */
    void backup();

    /**
     * @brief Hide sidebar and backup minimized state.
     * If there are other widgets in this splitter, stores
     * their sizes in the provided list.
     */
    void backup(const QList<QWidget*>& thirdWidgetsToBackup, QList<int>* const sizes);

    /**
     * @brief Show sidebar and restore minimized state.
     */
    void restore();

    /**
     * @brief Show sidebar and restore minimized state.
     * Restores other widgets' sizes in splitter.
     */
    void restore(const QList<QWidget*>& thirdWidgetsToRestore, const QList<int>& sizes);

    /**
     * @return the visible status of current sidebar tab.
     */
    bool isExpanded()                 const;

protected:

    /**
     * @brief Load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState()                      override;

    /**
     * @brief Save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState()                      override;

private:

    bool eventFilter(QObject* o, QEvent* e) override;
    void switchTabAndStackToTab(int tab);

private Q_SLOTS:

    /**
     * @brief Activates a tab
     */
    void slotClicked(int tab);

    void slotExpandTimer();
    void slotDragSwitchTimer();

    void slotSplitterBtnClicked();

Q_SIGNALS:

    /**
     * @brief Is emitted, when another tab is activated
     */
    void signalChangedTab(QWidget* w);

    /**
     * @brief Is emitted, when tab is shrink or expanded
     */
    void signalViewChanged();

private:

    friend class SidebarSplitter;

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT SidebarSplitter : public QSplitter
{
    Q_OBJECT

public:

    const static QString DEFAULT_CONFIG_KEY;

    /**
     * @brief This is a QSplitter with better support for storing its state
     * in config files, especially if Sidebars are contained in the splitter.
     */
    explicit SidebarSplitter(QWidget* const parent = nullptr);
    explicit SidebarSplitter(Qt::Orientation orientation, QWidget* const parent = nullptr);

    ~SidebarSplitter()                    override;

    /**
     * @brief Saves the splitter state to group, handling minimized sidebars correctly.
     * DEFAULT_CONFIG_KEY is used for storing the state.
     */
    void saveState(KConfigGroup& group);

    /**
     * @brief Saves the splitter state to group, handling minimized sidebars correctly.
     * This version uses a specified key in the config group.
     */
    void saveState(KConfigGroup& group, const QString& key);

    /**
     * @brief Restores the splitter state from group, handling minimized sidebars correctly.
     * DEFAULT_CONFIG_KEY is used for restoring the state.
     */
    void restoreState(KConfigGroup& group);

    /**
     * @brief Restores the splitter state from group, handling minimized sidebars correctly.
     * This version uses a specified key in the config group.
     */
    void restoreState(KConfigGroup& group, const QString& key);

    /**
     * @return the value of sizes() that corresponds to the given Sidebar or splitter child widget.
     */
    int size(Sidebar* const bar)    const;
    int size(QWidget* const widget) const;

    /**
     * @brief Sets the splitter size for the given sidebar or splitter child widget to size.
     * Special value -1: Sets the minimum size hint of the widget.
     */
    void setSize(Sidebar* const bar, int size);
    void setSize(QWidget* const widget, int size);

    void addSplitterCollapserButton(QWidget* const widget);

private Q_SLOTS:

    void slotSplitterMoved(int pos, int index);

private:

    friend class Sidebar;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
