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

namespace Marble
{

class GeoDataTimeStamp;
class GeoDataTimeSpanPrivate;

class DIGIKAM_EXPORT GeoDataTimeSpan : public GeoDataTimePrimitive
{
public:

    GeoDataTimeSpan();
    GeoDataTimeSpan(const GeoDataTimeSpan& other);
    ~GeoDataTimeSpan()                                        override;

    /**
     * @brief assignment operator
     */
    GeoDataTimeSpan& operator=(const GeoDataTimeSpan& other);

    /**
      * @brief equality operators
      */
    bool operator==(const GeoDataTimeSpan& other)       const;
    bool operator!=(const GeoDataTimeSpan& other)       const;

public:

    /// Provides type information for downcasting a GeoNode
    const char* nodeType()                              const override;

    /**
     * @brief return the beginning instant of a timespan
     */
    const GeoDataTimeStamp& begin()                     const;
    GeoDataTimeStamp& begin();

    /**
     * @brief Set the beginning instant of a timespan
     * @param begin the beginning instant of a timespan
     */
    void setBegin(const GeoDataTimeStamp& begin);

    /**
     * @brief return the ending instant of a timespan
     */
    const GeoDataTimeStamp& end()                       const;
    GeoDataTimeStamp& end();

    /**
     * @brief Set the ending instant of a timespan
     * @param end the ending instant of a timespan
     */
    void setEnd(const GeoDataTimeStamp& end);

    /**
     * @return True iff either of begin or end is valid, or if begin and end are both valid and begin is <= end
     */
    bool isValid()                                      const;

    /**
     * @brief Serialize the timespan to a stream
     * @param  stream  the stream
     */
    void pack(QDataStream& stream)                      const override;

    /**
     * @brief  Unserialize the timespan from a stream
     * @param  stream  the stream
     */
    void unpack(QDataStream& stream)                          override;

private:

    GeoDataTimeSpanPrivate* const d = nullptr;
};

} // namespace Marble
