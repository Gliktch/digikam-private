/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Rajce web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rajceplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "rajcewindow.h"

namespace DigikamGenericRajcePlugin
{

RajcePlugin::RajcePlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

RajcePlugin::~RajcePlugin()
{
}

void RajcePlugin::cleanUp()
{
    delete m_toolDlg;
}

QString RajcePlugin::name() const
{
    return i18nc("@title", "Rajce");
}

QString RajcePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RajcePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-rajce"));
}

QString RajcePlugin::description() const
{
    return i18nc("@info", "A tool to export to Rajce web-service");
}

QString RajcePlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to Rajce web-service.<br><br>"
                 "See Rajce web site for details: %1",
                 QLatin1String("<a href='https://www.rajce.idnes.cz/'>https://www.rajce.idnes.cz/</a>"));
}

QString RajcePlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString RajcePlugin::handbookChapter() const
{
    return QLatin1String("rajce_export");
}

QList<DPluginAuthor> RajcePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Lukas Krejci"),
                             QString::fromUtf8("metlosh at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2013"))
            ;
}

void RajcePlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Rajce..."));
    ac->setObjectName(QLatin1String("export_rajce"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_J);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRajce()));

    addAction(ac);
}

void RajcePlugin::slotRajce()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new RajceWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericRajcePlugin

#include "moc_rajceplugin.cpp"
