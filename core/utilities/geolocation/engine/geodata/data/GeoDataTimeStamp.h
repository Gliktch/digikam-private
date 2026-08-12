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

// Local includes

#include "GeoDataTimePrimitive.h"
#include "digikam_export.h"

class QDateTime;

namespace Marble
{

class GeoDataTimeStampPrivate;

class DIGIKAM_EXPORT GeoDataTimeStamp : public GeoDataTimePrimitive
{
public:

    enum TimeResolution
    {
        SecondResolution,
        DayResolution,
        MonthResolution,
        YearResolution
    };

    GeoDataTimeStamp();
    GeoDataTimeStamp(const GeoDataTimeStamp& other);
    ~GeoDataTimeStamp() override;

    /**
     * @brief assignment operator
     */
    GeoDataTimeStamp& operator=(const GeoDataTimeStamp& other);

    /**
     * @brief equality operators
     */
    bool operator==(const GeoDataTimeStamp& other) const;
    bool operator!=(const GeoDataTimeStamp& other) const;


    /// Provides type information for downcasting a GeoNode
    const char* nodeType() const override;

    /**
     * @brief return the when time of timestamp
     */
    QDateTime when() const;

    /**
     * @brief Set the when time of timestamp
     * @param when the when time of timestamp
     */
    void setWhen(const QDateTime& when);

    void setResolution(TimeResolution resolution);

    TimeResolution resolution() const;

    /**
     * @brief Serialize the timestamp to a stream
     * @param  stream  the stream
     */
    void pack(QDataStream& stream) const override;

    /**
     * @brief  Unserialize the timestamp from a stream
     * @param  stream  the stream
     */
    void unpack(QDataStream& stream) override;

private:

    GeoDataTimeStampPrivate* const d = nullptr;
};

} // namespace Marble
