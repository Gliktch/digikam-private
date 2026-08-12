/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-02
 * Description : plugin to export image as wallpaper.
 *
 * SPDX-FileCopyrightText: 2019      by Igor Antropov <antropovi at yahoo dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wallpaperplugin.h"

// Qt includes

#include <QPointer>
#include <QEventLoop>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "wallpaperplugindlg.h"

namespace DigikamGenericWallpaperPlugin
{

WallpaperPlugin::WallpaperPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

QString WallpaperPlugin::name() const
{
    return i18n("Export as wallpaper");
}

QString WallpaperPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WallpaperPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-desktop-wallpaper"));
}

QString WallpaperPlugin::description() const
{
    return i18n("A tool to set image as wallpaper");
}

QString WallpaperPlugin::details() const
{
    return i18n("<p>This tool changes background wallpaper to selected image for all desktops.</p>"
                "<p>If many images are selected, the first one will be used.</p>"
                "<p>If no image is selected, the first one from current album will be used.</p>");
}

QString WallpaperPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString WallpaperPlugin::handbookChapter() const
{
    return QLatin1String("wall_paper");
}

QList<DPluginAuthor> WallpaperPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Igor Antropov"),
                             QString::fromUtf8("antropovi at yahoo dot com"),
                             QString::fromUtf8("(C) 2019"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2025"),
                             i18n("Author and Maintainer"));
}

void WallpaperPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Set as wallpaper"));
    ac->setObjectName(QLatin1String("Wallpaper"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotWallpaper()));

    addAction(ac);
}

void WallpaperPlugin::slotWallpaper()
{
    const DInfoInterface* const iface = infoIface(sender());
    QList<QUrl> images                = iface->currentSelectedItems();

    if (images.isEmpty())
    {
        images = iface->currentAlbumItems();
    }

    if (!images.isEmpty())
    {

#ifndef Q_OS_MACOS

        QPointer<WallpaperPluginDlg> dlg = new WallpaperPluginDlg(this);
        dlg->show();

        QEventLoop loop;

        connect(dlg, &QDialog::finished,
                this, [this, &loop, dlg, images](int result)
            {
                if (result == QDialog::Accepted)
                {
                    setWallpaper(images[0].toString(), dlg->wallpaperLayout());
                }

                loop.quit();
                delete dlg;
            }
        );

        loop.exec();

#else

        setWallpaper(images[0].toString());

#endif

    }
}

} // namespace DigikamGenericWallpaperPlugin

#include "moc_wallpaperplugin.cpp"
