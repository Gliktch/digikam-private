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

#pragma once

// Qt includes

#include <QDebug>
#include <QFont>

// Local includes

#include "MarbleGlobal.h"
#include "MarbleLocale.h"
#include "digikam_export.h"

class KConfigGroup;

using namespace Marble;

namespace Digikam
{

/**
 * The class GeolocationSettingsContainer encapsulates all Marble related settings.
 */
class DIGIKAM_EXPORT GeolocationSettingsContainer
{
public:

    GeolocationSettingsContainer()  = default;
    ~GeolocationSettingsContainer() = default;

public:

    void readFromConfig(const KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

public:

    MarbleLocale::MeasurementSystem distanceUnit             = MarbleLocale::MetricSystem;

    Marble::AngleUnit               angleUnit                = Marble::DecimalDegree;

    Marble::MapQuality              stillQuality             = Marble::HighQuality;
    Marble::MapQuality              animationQuality         = Marble::LowQuality;

    QFont                           mapFont;

    bool                            inertialRotation         = true;
    bool                            mouseRotation            = true;

    int                             volatileTileCacheLimit   = 100;
    int                             persistentTileCacheLimit = 999999;

    bool                            showGrid                 = true;
    bool                            showCross                = true;
    bool                            showAtmos                = false;
    bool                            showSunShading           = false;
    bool                            showCities               = true;
    bool                            showRelief               = true;
};

//! qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const GeolocationSettingsContainer& inf);

} // namespace Digikam
