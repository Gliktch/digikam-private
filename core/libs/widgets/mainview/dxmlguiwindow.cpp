/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dxmlguiwindow_p.h"

namespace Digikam
{

DXmlGuiWindow::DXmlGuiWindow(QWidget* const parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f),
      d            (new Private)
{
    m_animLogo = nullptr;

    installEventFilter(this);

    QScreen* const screen = qApp->primaryScreen();
    int width             = screen->availableSize().width()  / 1.10;
    int height            = screen->availableSize().height() / 1.10;

    resize(width, height);
}

DXmlGuiWindow::~DXmlGuiWindow()
{
    delete d;
}

void DXmlGuiWindow::setConfigGroupName(const QString& name)
{
    d->configGroupName = name;
}

QString DXmlGuiWindow::configGroupName() const
{
    return d->configGroupName;
}

void DXmlGuiWindow::closeEvent(QCloseEvent* e)
{
    checkFullScreenBeforeClosing();

    if (!testAttribute(Qt::WA_DeleteOnClose))
    {
        e->ignore();

        return;
    }

    KXmlGuiWindow::closeEvent(e);
}

bool DXmlGuiWindow::slotClose()
{
    checkFullScreenBeforeClosing();

    return close();
}

void DXmlGuiWindow::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void DXmlGuiWindow::slotShowStatusBar()
{
    statusBar()->setVisible(d->showStatusBarAction->isChecked());
}

void DXmlGuiWindow::slotConfNotifications()
{

#ifdef HAVE_KNOTIFYCONFIG

    QDialog* const dialog             = new QDialog(this);
    dialog->setWindowTitle(i18n("Configure Notifications"));

    KNotifyConfigWidget* const w      = new KNotifyConfigWidget(dialog);

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel| QDialogButtonBox::Help);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    QVBoxLayout* const layout         = new QVBoxLayout(dialog);
    layout->addWidget(w);
    layout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            w, SLOT(save()));

    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            w, SLOT(save()));

    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked,
            this, []()
        {
            openOnlineDocumentation(QLatin1String("setup_application"),
                                    QLatin1String("notifications_settings"));
        }
    );

    connect(w, SIGNAL(changed(bool)),
            buttonBox->button(QDialogButtonBox::Apply), SLOT(setEnabled(bool)));

    connect(buttonBox, SIGNAL(accepted()),
            dialog, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            dialog, SLOT(reject()));

    w->setApplication(QString());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

#endif

}

void DXmlGuiWindow::editKeyboardShortcuts(KActionCollection* const extraac, const QString& actitle)
{
    QPointer<KShortcutsDialog> dialog = new KShortcutsDialog(KShortcutsEditor::AllActions,
                                                             KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog->addCollection(actionCollection(), i18nc("general keyboard shortcuts", "General"));

    QDialogButtonBox* const btnbox = dialog->findChild<QDialogButtonBox*>();

    if (btnbox)
    {
        btnbox->setStandardButtons(btnbox->standardButtons() | QDialogButtonBox::Help);

        connect(btnbox->button(QDialogButtonBox::Help), &QPushButton::clicked,
                this, []()
            {
                openOnlineDocumentation(QLatin1String("setup_application"),
                                        QLatin1String("shortcuts_settings"));
            }
        );
    }

    if (extraac)
    {
        dialog->addCollection(extraac, actitle);
    }

    dialog->configure();
}

bool DXmlGuiWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == this)
    {
        if (ev && (ev->type() == QEvent::HoverMove) && fullScreenIsActive())
        {
            // We will handle a stand alone FullScreen button action on top/right corner of screen
            // only if managed window tool bar is hidden, and if we switched already in Full Screen mode.

            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                if (((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars) || !mainbar->isVisible())
                {
                    QHoverEvent* const mev = dynamic_cast<QHoverEvent*>(ev);

                    if (mev)
                    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                        QPoint pos(mev->position().toPoint());

#else

                        QPoint pos(mev->pos());

#endif

                        QRect  desktopRect = windowHandle()->screen()->geometry();

                        QRect sizeRect(QPoint(0, 0), d->fullScreenBtn->size());
                        QRect topLeft, topRight;
                        QRect topRightLarger;

                        desktopRect        = QRect(desktopRect.y(), desktopRect.y(), desktopRect.width(), desktopRect.height());
                        topLeft            = sizeRect;
                        topRight           = sizeRect;

                        topLeft.moveTo(desktopRect.x(), desktopRect.y());
                        topRight.moveTo(desktopRect.x() + desktopRect.width() - sizeRect.width() - 1, topLeft.y());

                        topRightLarger     = topRight.adjusted(-25, 0, 0, 10);

                        if (topRightLarger.contains(pos))
                        {
                            d->fullScreenBtn->move(topRight.topLeft());
                            d->fullScreenBtn->show();
                        }
                        else
                        {
                            d->fullScreenBtn->hide();
                        }

                        return false;
                    }
                }
            }
        }
    }

    // pass the event on to the parent class

    return QObject::eventFilter(obj, ev);
}

void DXmlGuiWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (fullScreenIsActive())
        {
            d->fullScreenAction->activate(QAction::Trigger);
        }
    }
}

void DXmlGuiWindow::showSideBars(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::showThumbBar(bool visible)
{
    Q_UNUSED(visible);
}

bool DXmlGuiWindow::thumbbarVisibility() const
{
    return true;
}

void DXmlGuiWindow::slotHelpContents()
{
    openOnlineDocumentation(d->handbookSection);
}

void DXmlGuiWindow::unminimizeAndActivateWindow()
{
    if (isMinimized())
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }

    raise();
    windowHandle()->requestActivate();
}

void DXmlGuiWindow::restoreWindowSize(QWindow* const win, const KConfigGroup& group)
{
    KWindowConfig::restoreWindowSize(win, group);
}

void DXmlGuiWindow::saveWindowSize(QWindow* const win, KConfigGroup& group)
{
    KWindowConfig::saveWindowSize(win, group);
}

void DXmlGuiWindow::setGoodDefaultWindowSize(QWindow* const win)
{
    QScreen* const screen = win->screen();
    int sw                = screen->availableSize().width();
    int sh                = screen->availableSize().height();
    int width             = qRound(log10(sw / 80) * 800);
    int height            = qRound(log10(sh / 60) * 600);

    win->resize((width  > sw) ? sw : width,
                (height > sh) ? sh : height);
}

void DXmlGuiWindow::slotRawCameraList()
{
    showRawCameraList();
}

void DXmlGuiWindow::slotSolidHardwareList()
{
    SolidHardwareDlg* const dlg = new SolidHardwareDlg(qApp->activeWindow());
    dlg->open();
}

void DXmlGuiWindow::slotDonateMoney()
{
    QUrl url(QLatin1String("https://www.digikam.org/donate/"));

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::slotRecipesBook()
{
    QUrl url(QLatin1String("https://www.digikam.org/recipes_book/"));

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::slotContribute()
{
    QUrl url(QLatin1String("https://www.digikam.org/contribute/"));

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();
}

void DXmlGuiWindow::setupIconTheme()
{
    // Let QStandardPaths handle this, it will look for app local stuff
    // this means e.g. for mac: "<APPDIR>/../Resources" and for win: "<APPDIR>/data".

    bool hasBreeze                = false;
    const QString breezeIcons     = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                           QLatin1String("breeze.rcc"));

    if (!breezeIcons.isEmpty() && QFile::exists(breezeIcons))
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Breeze icons resource file found";
        QResource::registerResource(breezeIcons);
        hasBreeze = true;
    }

    bool hasBreezeDark            = false;
    const QString breezeDarkIcons = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                           QLatin1String("breeze-dark.rcc"));

    if (!breezeDarkIcons.isEmpty() && QFile::exists(breezeDarkIcons))
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Breeze-dark icons resource file found";
        QResource::registerResource(breezeDarkIcons);
        hasBreezeDark = true;
    }

    if (hasBreeze || hasBreezeDark)
    {
        // Tell Qt about the theme

        QIcon::setThemeSearchPaths(QStringList() << QLatin1String(":/icons"));

        const QStringList iconsDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                                QLatin1String("icons"),
                                                                QStandardPaths::LocateDirectory);

        if (!iconsDirs.isEmpty())
        {
            QIcon::setThemeSearchPaths(QStringList() << QIcon::themeSearchPaths() << iconsDirs);
        }

        if      (hasBreeze)
        {
            QIcon::setThemeName(QLatin1String("breeze"));
        }
        else if (hasBreezeDark)
        {
            QIcon::setThemeName(QLatin1String("breeze-dark"));
        }
    }
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Use installed icons";
    }
}

} // namespace Digikam

#include "moc_dxmlguiwindow.cpp"
