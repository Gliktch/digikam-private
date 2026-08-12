/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class MarbleLocalePrivate;

/**
 * @short A class that contains all localization stuff for Marble.
 *
 * The class stores properties like the measurement system.
 */
class DIGIKAM_EXPORT MarbleLocale
{

public:

    MarbleLocale();
    ~MarbleLocale();

    enum MeasurementSystem
    {
        MetricSystem = 0,
        ImperialSystem,
        NauticalSystem
    };

    enum MeasureUnit
    {
        Meter = 0,
        Kilometer,
        Millimeter,
        Centimeter,

        Foot,
        Inch,
        Yard,
        Mile,

        NauticalMile
    };

    void setMeasurementSystem(MarbleLocale::MeasurementSystem measurementSystem);
    MarbleLocale::MeasurementSystem measurementSystem() const;

    static QString languageCode();

    /**
     * @brief meterToCustomUnit Converts @p meters to @p targetValue of @p targetUnit in @p targetSystem
     */
    static void meterToTargetUnit(qreal meters, MeasurementSystem targetSystem,
                                  qreal& targetValue, MeasureUnit& targetUnit);

    /**
     * @brief unitAbbreviation Represents a short string form of @p unit, e.g. "km" for Kilometer
     */
    static QString unitAbbreviation(MeasureUnit unit);

private:

    Q_DISABLE_COPY(MarbleLocale)

private:

    MarbleLocalePrivate* const d = nullptr;
};

} // Namespace Marble
