/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-02
 * Description : plugin to export image as wallpaper - DBUS version.
 * References  : https://bugs.kde.org/show_bug.cgi?id=217950
 *
 * SPDX-FileCopyrightText: 2019      by Igor Antropov <antropovi at yahoo dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wallpaperplugin.h"

// Qt includes

#include <QMessageBox>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericWallpaperPlugin
{

bool WallpaperPlugin::setWallpaper(const QString& path, int layout) const
{
    QDBusMessage message = QDBusMessage::createMethodCall
        (
            QLatin1String("org.kde.plasmashell"),
            QLatin1String("/PlasmaShell"),
            QLatin1String("org.kde.PlasmaShell"),
            QLatin1String("evaluateScript")
        );

    /**
     * Example of WallPaper settings from plasma-org.kde.plasma.desktop-appletsrc:
     *
     * [Containments][1][Wallpaper][org.kde.image][General]
     * FillMode=2
     * Image=file:///home/gilles/Images/M104_left.jpg
     * height=1440
     * width=2560
     *
     * FillMode can take these values:
     *    0 = Adjusted
     *    1 = Adjusted with aspect ratio
     *    2 = Adjusted and cropped      (Stretch)
     *    3 = Mosaic                    (Tile)
     *    4 = ???
     *    5 = ???
     *    6 = Centered                  (Center)
     *
     * [Containments][1] = screen 1
     * [Containments][2] = screen 2
     * [Containments][3] = screen 3
     * etc.
     */

    message << QString::fromUtf8
        (
            "var allDesktops = desktops();"
            "for (i=0;i<allDesktops.length;i++)"
            "{"
                "d = allDesktops[i];"
                "d.wallpaperPlugin = \"org.kde.image\";"
                "d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\");"
                "d.writeConfig(\"Image\", \"%1\");"
                "d.writeConfig(\"FillMode\", \"%2\")"
            "}"
        )
        .arg(path)
        .arg(layout);

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        QMessageBox::warning(nullptr,
                             i18nc("@title:window",
                                   "Error While to Set Image as Wallpaper"),
                             i18nc("@info", "Cannot change wallpaper image from current desktop with\n"
                                            "%1\n\nError: %2",
                                   path,
                                   reply.errorMessage()));

        return false;
    }

    return true;
}

} // namespace DigikamGenericWallpaperPlugin
