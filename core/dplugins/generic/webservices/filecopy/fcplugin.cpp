/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a plugin to export items to a local storage.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fcplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "fcexportwindow.h"

namespace DigikamGenericFileCopyPlugin
{

FCPlugin::FCPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void FCPlugin::cleanUp()
{
    delete m_toolDlgExport;
}

QString FCPlugin::name() const
{
    return i18nc("@title", "File Copy");
}

QString FCPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FCPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("drive-removable-media"));
}

QString FCPlugin::description() const
{
    return i18nc("@info", "A tool to export items to a local storage");
}

QString FCPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to a local storage.");
}

QString FCPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString FCPlugin::handbookChapter() const
{
    return QLatin1String("file_copy");
}

QList<DPluginAuthor> FCPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Johannes Wienke"),
                             QString::fromUtf8("languitar at semipol dot de"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2025"))
            << DPluginAuthor(QString::fromUtf8("Marcel Mathis"),
                             QString::fromUtf8("maeseee at gmail dot com"),
                             QString::fromUtf8("(C) 2020"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2025"))
            ;
}

void FCPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to local storage..."));
    ac->setObjectName(QLatin1String("export_filecopy"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_L);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFileCopyExport()));

    addAction(ac);
}

void FCPlugin::slotFileCopyExport()
{
    if (!reactivateToolDialog(m_toolDlgExport))
    {
        delete m_toolDlgExport;
        m_toolDlgExport = new FCExportWindow(infoIface(sender()), nullptr);
        m_toolDlgExport->setPlugin(this);
        m_toolDlgExport->show();
    }
}

} // namespace DigikamGenericFileCopyPlugin

#include "moc_fcplugin.cpp"
