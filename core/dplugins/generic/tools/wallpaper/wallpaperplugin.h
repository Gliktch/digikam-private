/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-02
 * Description : plugin to export image as wallpaper
 *
 * SPDX-FileCopyrightText: 2019      by Igor Antropov <antropovi at yahoo dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QIcon>
#include <QList>

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.Wallpaper"

using namespace Digikam;

namespace DigikamGenericWallpaperPlugin
{

class WallpaperPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    /**
     * Default values for Plasma Wallpaper plugin.
     */
    enum WallpaperLayout
    {
        Adjusted            = 0,
        AdjustedAspectRatio = 1,
        AdjustedCropped     = 2,
        Mosaic              = 3,
        Centered            = 6
    };

public:

    explicit WallpaperPlugin(QObject* const parent = nullptr);
    ~WallpaperPlugin()                   override = default;

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;

    void setup(QObject* const)           override;

private Q_SLOTS:

    void slotWallpaper();

private:

    bool setWallpaper(const QString& path, int layout = AdjustedCropped) const;
};

} // namespace DigikamGenericWallpaperPlugin
