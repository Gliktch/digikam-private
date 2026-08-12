/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window - Actions methods.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dxmlguiwindow_p.h"

namespace Digikam
{

void DXmlGuiWindow::registerPluginsActions()
{
    guiFactory()->removeClient(this);

    DPluginLoader* const dpl      = DPluginLoader::instance();
    dpl->registerGenericPlugins(this);

    QList<DPluginAction*> actions = dpl->pluginsActions(DPluginAction::Generic, this);

    for (DPluginAction* const ac : std::as_const(actions))
    {
        actionCollection()->addActions(QList<QAction*>() << ac);
        actionCollection()->setDefaultShortcuts(ac, ac->shortcuts());
    }

    QString dom = domDocument().toString();
    dom.replace(QLatin1String("<!-- _DPLUGINS_GENERIC_TOOL_ACTIONS_ -->"),     dpl->pluginXmlSections(DPluginAction::GenericTool,     this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_GENERIC_METADATA_ACTIONS_ -->"), dpl->pluginXmlSections(DPluginAction::GenericMetadata, this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_GENERIC_IMPORT_ACTIONS_ -->"),   dpl->pluginXmlSections(DPluginAction::GenericImport,   this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_GENERIC_EXPORT_ACTIONS_ -->"),   dpl->pluginXmlSections(DPluginAction::GenericExport,   this));
    dom.replace(QLatin1String("<!-- _DPLUGINS_GENERIC_VIEW_ACTIONS_ -->"),     dpl->pluginXmlSections(DPluginAction::GenericView,     this));

    registerExtraPluginsActions(dom);

    setXML(dom);

    guiFactory()->reset();
    guiFactory()->addClient(this);

    checkAmbiguousShortcuts();
}

void DXmlGuiWindow::createHelpActions(const QString& handbookSection, bool coreOptions)
{
    d->handbookSection                  = handbookSection;

    d->libsInfoAction                   = new QAction(QIcon::fromTheme(QLatin1String("help-about")),
                                                                       i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction(QLatin1String("help_librariesinfo"), d->libsInfoAction);

    d->about                            = new DAboutData(this);

    QAction* const rawCameraListAction  = new QAction(QIcon::fromTheme(QLatin1String("image-x-adobe-dng")),
                                                                       i18n("Supported RAW Cameras"), this);
    connect(rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction(QLatin1String("help_rawcameralist"), rawCameraListAction);

    QAction* const solidHardwareAction  = new QAction(QIcon::fromTheme(QLatin1String("preferences-devices-tree")),
                                                                       i18n("List of Detected Hardware"), this);
    connect(solidHardwareAction, SIGNAL(triggered()), this, SLOT(slotSolidHardwareList()));
    actionCollection()->addAction(QLatin1String("help_solidhardwarelist"), solidHardwareAction);

    QAction* const donateMoneyAction    = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                                                       i18n("Donate..."), this);
    connect(donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction(QLatin1String("help_donatemoney"), donateMoneyAction);

    QAction* const recipesBookAction    = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                                                       i18n("Recipes Book..."), this);
    connect(recipesBookAction, SIGNAL(triggered()), this, SLOT(slotRecipesBook()));
    actionCollection()->addAction(QLatin1String("help_recipesbook"), recipesBookAction);

    QAction* const contributeAction     = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                                                       i18n("Contribute..."), this);
    connect(contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction(QLatin1String("help_contribute"), contributeAction);

    QAction* const onlineVerCheckAction = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                                                       i18n("Check for New Version..."), this);
    connect(onlineVerCheckAction, SIGNAL(triggered()), this, SLOT(slotOnlineVersionCheck()));
    actionCollection()->addAction(QLatin1String("help_onlineversioncheck"), onlineVerCheckAction);

    QAction* const helpAction           = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                                                       i18n("Online Handbook..."), this);
    connect(helpAction, SIGNAL(triggered()), this, SLOT(slotHelpContents()));
    actionCollection()->addAction(QLatin1String("help_handbook"), helpAction);

    m_animLogo                          = new DLogoAction(this);
    actionCollection()->addAction(QLatin1String("logo_action"), m_animLogo);

    // Add options only for core components (typically all excepted Showfoto)

    if (coreOptions)
    {
        d->dbStatAction = new QAction(QIcon::fromTheme(QLatin1String("network-server-database")),
                                                       i18n("Database Statistics"), this);
        connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
        actionCollection()->addAction(QLatin1String("help_dbstat"), d->dbStatAction);
    }
}

void DXmlGuiWindow::cleanupActions()
{
    QAction* ac = actionCollection()->action(QLatin1String("help_about_kde"));
    if (ac) actionCollection()->removeAction(ac);

    ac          = actionCollection()->action(QLatin1String("help_donate"));
    if (ac) actionCollection()->removeAction(ac);

    ac          = actionCollection()->action(QLatin1String("help_contents"));
    if (ac) actionCollection()->removeAction(ac);

/*
    const auto acol = actionCollection()->actions();

    for (QAction* const act : acol)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "action: " << act->objectName();
    }
*/
    // Fix the animation logo action style.

    QWidget* const widget = mainToolBar()->widgetForAction(m_animLogo);

    if (QToolButton* const button = qobject_cast<QToolButton*>(widget))
    {
        DLogoAction::applyStyleForToolBarButton(button);
    }
}

void DXmlGuiWindow::createSidebarActions()
{
    KActionCollection* const ac = actionCollection();
    QAction* const tlsb         = new QAction(i18n("Toggle Left Side-bar"), this);
    connect(tlsb, SIGNAL(triggered()), this, SLOT(slotToggleLeftSideBar()));
    ac->addAction(QLatin1String("toggle-left-sidebar"), tlsb);
    ac->setDefaultShortcut(tlsb, Qt::CTRL | Qt::ALT | Qt::Key_Left);

    QAction* const trsb = new QAction(i18n("Toggle Right Side-bar"), this);
    connect(trsb, SIGNAL(triggered()), this, SLOT(slotToggleRightSideBar()));
    ac->addAction(QLatin1String("toggle-right-sidebar"), trsb);
    ac->setDefaultShortcut(trsb, Qt::CTRL | Qt::ALT | Qt::Key_Right);

    QAction* const plsb = new QAction(i18n("Previous Left Side-bar Tab"), this);
    connect(plsb, SIGNAL(triggered()), this, SLOT(slotPreviousLeftSideBarTab()));
    ac->addAction(QLatin1String("previous-left-sidebar-tab"), plsb);
    ac->setDefaultShortcut(plsb, Qt::CTRL | Qt::ALT | Qt::Key_Home);

    QAction* const nlsb = new QAction(i18n("Next Left Side-bar Tab"), this);
    connect(nlsb, SIGNAL(triggered()), this, SLOT(slotNextLeftSideBarTab()));
    ac->addAction(QLatin1String("next-left-sidebar-tab"), nlsb);
    ac->setDefaultShortcut(nlsb, Qt::CTRL | Qt::ALT | Qt::Key_End);

    QAction* const prsb = new QAction(i18n("Previous Right Side-bar Tab"), this);
    connect(prsb, SIGNAL(triggered()), this, SLOT(slotPreviousRightSideBarTab()));
    ac->addAction(QLatin1String("previous-right-sidebar-tab"), prsb);
    ac->setDefaultShortcut(prsb, Qt::CTRL | Qt::ALT | Qt::Key_PageUp);

    QAction* const nrsb = new QAction(i18n("Next Right Side-bar Tab"), this);
    connect(nrsb, SIGNAL(triggered()), this, SLOT(slotNextRightSideBarTab()));
    ac->addAction(QLatin1String("next-right-sidebar-tab"), nrsb);
    ac->setDefaultShortcut(nrsb, Qt::CTRL | Qt::ALT | Qt::Key_PageDown);
}

void DXmlGuiWindow::createSettingsActions()
{
    d->showMenuBarAction   = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

#ifdef Q_OS_MACOS

    // Under macOS the menu bar visibility is managed by desktop.

    d->showMenuBarAction->setVisible(false);

#endif

    d->showStatusBarAction = actionCollection()->action(QLatin1String("options_show_statusbar"));

    if (!d->showStatusBarAction)
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Status bar menu action cannot be found in action collection";

        d->showStatusBarAction = new QAction(i18n("Show Statusbar"), this);
        d->showStatusBarAction->setCheckable(true);
        d->showStatusBarAction->setChecked(true);
        connect(d->showStatusBarAction, SIGNAL(toggled(bool)), this, SLOT(slotShowStatusBar()));
        actionCollection()->addAction(QLatin1String("options_show_statusbar"), d->showStatusBarAction);
    }

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());

#ifdef HAVE_KNOTIFYCONFIG

    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());

#endif

#if KCONFIGWIDGETS_VERSION > QT_VERSION_CHECK(5, 80, 0)

    d->hamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, actionCollection());
    d->hamburgerMenu->setShowMenuBarAction(d->showMenuBarAction);
    d->hamburgerMenu->setMenuBar(menuBar());
/*
    connect(d->hamburgerMenu, &KHamburgerMenu::aboutToShowMenu,
            this, [this]()
            {
                // Immediately disconnect. We only need to run this once, but on demand.
                // NOTE: The nullptr at the end disconnects all connections between
                // this and d->hamburgerMenu's aboutToShowMenu signal.

                disconnect(d->hamburgerMenu, &KHamburgerMenu::aboutToShowMenu,
                           this, nullptr);
            }
    );
*/
#endif

}

QAction* DXmlGuiWindow::showMenuBarAction() const
{
    return d->showMenuBarAction;
}

QAction* DXmlGuiWindow::showStatusBarAction() const
{
    return d->showStatusBarAction;
}

QAction* DXmlGuiWindow::buildStdAction(StdActionType type, const QObject* const recvr,
                                       const char* const slot, QObject* const parent)
{
    switch (type)
    {
        case StdCopyAction:
        {
            return KStandardAction::copy(recvr, slot, parent);
        }

        case StdPasteAction:
        {
            return KStandardAction::paste(recvr, slot, parent);
        }

        case StdCutAction:
        {
            return KStandardAction::cut(recvr, slot, parent);
        }

        case StdQuitAction:
        {
            return KStandardAction::quit(recvr, slot, parent);
        }

        case StdCloseAction:
        {
            return KStandardAction::close(recvr, slot, parent);
        }

        case StdZoomInAction:
        {
            return KStandardAction::zoomIn(recvr, slot, parent);
        }

        case StdZoomOutAction:
        {
            return KStandardAction::zoomOut(recvr, slot, parent);
        }

        case StdOpenAction:
        {

#ifndef __clang_analyzer__

            // NOTE: disable false positive report from scan build about std::open()

            return KStandardAction::open(recvr, slot, parent);

#endif

        }

        case StdSaveAction:
        {
            return KStandardAction::save(recvr, slot, parent);
        }

        case StdSaveAsAction:
        {
            return KStandardAction::saveAs(recvr, slot, parent);
        }

        case StdRevertAction:
        {
            return KStandardAction::revert(recvr, slot, parent);
        }

        case StdBackAction:
        {
            return KStandardAction::back(recvr, slot, parent);
        }

        case StdForwardAction:
        {
            return KStandardAction::forward(recvr, slot, parent);
        }

        default:
        {
            break;
        }
    }

    return nullptr;
}

QList<QAction*> DXmlGuiWindow::allActions() const
{
    return (actionCollection()->actions());
}

} // namespace Digikam
