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

#include "GeoDataObject.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataTimePrimitivePrivate;

/**
 * @short a base class for the style classes
 *
 * A GeoDataTimePrimitive is a base class for the time classes TimeSpan
 * and TimeStamp classes.
 *
 * @see GeoDataTimeSpan
 * @see GeoDataTimeStamp
 */
class DIGIKAM_EXPORT GeoDataTimePrimitive : public GeoDataObject
{
public:

    /// Provides type information for downcasting a GeoNode
    const char* nodeType() const override;

    GeoDataTimePrimitive& operator=(const GeoDataTimePrimitive& other);

    /**
     * @brief Serialize the styleselector to a stream
     * @param  stream  the stream
     */
    void pack(QDataStream& stream) const override;          // cppcheck-suppress uselessOverride

    /**
     * @brief  Unserialize the styleselector from a stream
     * @param  stream  the stream
     */
    void unpack(QDataStream& stream) override;              // cppcheck-suppress uselessOverride

    GeoDataTimePrimitive();
    GeoDataTimePrimitive(const GeoDataTimePrimitive& other);
    ~GeoDataTimePrimitive() override;

private:

    GeoDataTimePrimitivePrivate* const d = nullptr;
};

} // namespace Marble
