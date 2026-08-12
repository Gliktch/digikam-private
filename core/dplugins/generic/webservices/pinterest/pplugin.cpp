/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Pinterest web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "pplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "pwindow.h"

namespace DigikamGenericPinterestPlugin
{

PPlugin::PPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void PPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString PPlugin::name() const
{
    return i18nc("@title", "Pinterest");
}

QString PPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-pinterest"));
}

QString PPlugin::description() const
{
    return i18nc("@info", "A tool to export to Pinterest web-service");
}

QString PPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to Pinterest web-service.<br><br>"
                 "See Pinterest web site for details: %1",
                 QLatin1String("<a href='https://www.pinterest.com/'>https://www.pinterest.com/</a>"));
}

QString PPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString PPlugin::handbookChapter() const
{
    return QLatin1String("pinterest_export");
}

QList<DPluginAuthor> PPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Tarek Talaat"),
                             QString::fromUtf8("tarektalaat93 at gmail dot com"),
                             QString::fromUtf8("(C) 2018"))
            ;
}

void PPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Pinterest..."));
    ac->setObjectName(QLatin1String("export_pinterest"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_I);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPinterest()));

    addAction(ac);
}

void PPlugin::slotPinterest()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new PWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericPinterestPlugin

#include "moc_pplugin.cpp"
