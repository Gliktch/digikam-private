/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to ImageShack web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageshackplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageshackwindow.h"

namespace DigikamGenericImageShackPlugin
{

ImageShackPlugin::ImageShackPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void ImageShackPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString ImageShackPlugin::name() const
{
    return i18nc("@title", "ImageShack");
}

QString ImageShackPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ImageShackPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-imageshack"));
}

QString ImageShackPlugin::description() const
{
    return i18nc("@info", "A tool to export to ImageShack web-service");
}

QString ImageShackPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to ImageShack web-service.\n\n"
                 "See ImageShack web site for details: %1",
                 QLatin1String("<a href='https://imageshack.us/'>https://imageshack.us/</a>"));
}

QString ImageShackPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString ImageShackPlugin::handbookChapter() const
{
    return QLatin1String("image_shack");
}

QList<DPluginAuthor> ImageShackPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Dodon Victor"),
                             QString::fromUtf8("dodonvictor at gmail dot com"),
                             QString::fromUtf8("(C) 2012"))
            ;
}

void ImageShackPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Imageshack..."));
    ac->setObjectName(QLatin1String("export_imageshack"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_M);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotImageShack()));

    addAction(ac);
}

void ImageShackPlugin::slotImageShack()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new ImageShackWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericImageShackPlugin

#include "moc_imageshackplugin.cpp"
