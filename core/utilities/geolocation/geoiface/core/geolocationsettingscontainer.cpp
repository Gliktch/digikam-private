/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-21
 * Description : Geolocation Settings Container.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "geolocationsettingscontainer.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "geolocationsettings.h"

namespace Digikam
{

void GeolocationSettingsContainer::readFromConfig(const KConfigGroup& group)
{
    distanceUnit             = (MarbleLocale::MeasurementSystem)group.readEntry("Distance Unit",     (int)MarbleLocale::MetricSystem);
    angleUnit                = (Marble::AngleUnit)              group.readEntry("Angle Unit",        (int)Marble::DecimalDegree);
    stillQuality             = (Marble::MapQuality)             group.readEntry("Still Quality",     (int)Marble::HighQuality);
    animationQuality         = (Marble::MapQuality)             group.readEntry("Animation Quality", (int)Marble::LowQuality);
    mapFont                  = group.readEntry("Map Font",                    QFont());
    inertialRotation         = group.readEntry("Inertial Rotation",           true);
    mouseRotation            = group.readEntry("Mouse Rotation",              true);
    volatileTileCacheLimit   = group.readEntry("Volatile Tile Cache Limit",   100);
    persistentTileCacheLimit = group.readEntry("Persistent Tile Cache Limit", 999999);
    showGrid                 = group.readEntry("Show Grid",                   true);
    showCross                = group.readEntry("Show Cross",                  true);
    showCross                = group.readEntry("Show Atmosphere",             false);
    showSunShading           = group.readEntry("Show Sun Shading",            false);
    showCities               = group.readEntry("Show Cities",                 true);
    showRelief               = group.readEntry("Show Relief",                 true);
}

void GeolocationSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("Distance Unit",               (int)distanceUnit);
    group.writeEntry("Angle Unit",                  (int)angleUnit);
    group.writeEntry("Still Quality",               (int)stillQuality);
    group.writeEntry("Animation Quality",           (int)animationQuality);
    group.writeEntry("Map Font",                    mapFont);
    group.writeEntry("Inertial Rotation",           (bool)inertialRotation);
    group.writeEntry("Mouse Rotation",              (bool)mouseRotation);
    group.writeEntry("Volatile Tile Cache Limit",   volatileTileCacheLimit);
    group.writeEntry("Persistent Tile Cache Limit", persistentTileCacheLimit);
    group.writeEntry("Show Grid",                   (bool)showGrid);
    group.writeEntry("Show Cross",                  (bool)showCross);
    group.writeEntry("Show Atmosphere",             (bool)showAtmos);
    group.writeEntry("Show Sun Shading",            (bool)showSunShading);
    group.writeEntry("Show Cities",                 (bool)showCities);
    group.writeEntry("Show Relief",                 (bool)showRelief);
}

QDebug operator<<(QDebug dbg, const GeolocationSettingsContainer& inf)
{
    dbg.nospace() << "[GeolocationSettingsContainer]"       << Qt::endl;
    dbg.nospace() << "   distanceUnit("
                  << inf.distanceUnit << "), "              << Qt::endl;
    dbg.nospace() << "   angleUnit("
                  << inf.angleUnit << "), "                 << Qt::endl;
    dbg.nospace() << "   stillQuality("
                  << inf.stillQuality << "), "              << Qt::endl;
    dbg.nospace() << "   animationQuality("
                  << inf.animationQuality << "), "          << Qt::endl;
    dbg.nospace() << "   mapFont("
                  << inf.mapFont << "), "                   << Qt::endl;
    dbg.nospace() << "   inertialRotation("
                  << inf.inertialRotation << "), "          << Qt::endl;
    dbg.nospace() << "   mouseRotation("
                  << inf.mouseRotation << "), "             << Qt::endl;
    dbg.nospace() << "   volatileTileCacheLimit("
                  << inf.volatileTileCacheLimit << "), "    << Qt::endl;
    dbg.nospace() << "   persistentTileCacheLimit("
                  << inf.persistentTileCacheLimit << "), "  << Qt::endl;
    dbg.nospace() << "   showGrid("
                  << inf.showGrid << "), "                  << Qt::endl;
    dbg.nospace() << "   showCross("
                  << inf.showCross << "), "                 << Qt::endl;
    dbg.nospace() << "   showAtmosphere("
                  << inf.showAtmos << "), "                 << Qt::endl;
    dbg.nospace() << "   showSunShading("
                  << inf.showSunShading << "), "            << Qt::endl;
    dbg.nospace() << "   showCities("
                  << inf.showCities << "), "                << Qt::endl;
    dbg.nospace() << "   showRelief("
                  << inf.showRelief << ")";

    return dbg.space();
}

} // namespace Digikam
