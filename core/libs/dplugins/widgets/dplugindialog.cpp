/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin main dialog
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dplugindialog.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QPointer>
#include <QMenu>
#include <QIcon>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"
#include "dpluginaboutdlg.h"
#include "dxmlguiwindow.h"
#include "dplugin.h"

namespace Digikam
{

DPluginDialog::DPluginDialog(QWidget* const parent, const QString& objName)
    : QDialog(parent)
{
    setObjectName(objName);
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint   |
                   Qt::WindowContextHelpButtonHint);

    m_buttons = new QDialogButtonBox(this);
}

DPluginDialog::~DPluginDialog()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(objectName());

    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
}

void DPluginDialog::setPlugin(DPlugin* const tool)
{
    m_tool = tool;

    if (m_tool)
    {
        QPushButton* const help       = m_buttons->addButton(QDialogButtonBox::Help);
        help->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
        help->setText(i18nc("@action: button", "Help"));
        help->setAutoDefault(false);
        QMenu* const menu             = new QMenu(help);
        QAction* const handbookAction = menu->addAction(QIcon::fromTheme(QLatin1String("globe")), i18n("Online Handbook..."));
        QAction* const aboutAction    = menu->addAction(QIcon::fromTheme(QLatin1String("help-about")), i18n("About..."));
        help->setMenu(menu);

        connect(handbookAction, SIGNAL(triggered()),
                this, SLOT(slotOnlineHandbook()));

        connect(aboutAction, SIGNAL(triggered()),
                this, SLOT(slotAboutPlugin()));
    }
}

void DPluginDialog::slotAboutPlugin()
{
    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(m_tool);
    dlg->exec();
    delete dlg;
}

void DPluginDialog::slotOnlineHandbook()
{
    openOnlineDocumentation(m_tool->handbookSection(),
                            m_tool->handbookChapter(),
                            m_tool->handbookReference());
}

void DPluginDialog::showEvent(QShowEvent* e)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(objectName());

    DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    QDialog::showEvent(e);
}

} // namespace Digikam

#include "moc_dplugindialog.cpp"
