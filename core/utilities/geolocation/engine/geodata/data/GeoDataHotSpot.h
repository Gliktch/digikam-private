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

#include <QPointF>

// Local includes

#include "GeoDataObject.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataHotSpotPrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataHotSpot : public GeoDataObject
{
public:

    enum Units
    {
        Fraction,
        Pixels,
        InsetPixels
    };

public:

    explicit GeoDataHotSpot(const QPointF& hotSpot = QPointF(0.5, 0.5),
                            Units xunits = Fraction, Units yunits = Fraction);
    GeoDataHotSpot(const GeoDataHotSpot& other);
    ~GeoDataHotSpot()                                                 override;

    GeoDataHotSpot& operator=(const GeoDataHotSpot& other);
    bool operator==(const GeoDataHotSpot& other)                const;
    bool operator!=(const GeoDataHotSpot& other)                const;

    /// Provides type information for downcasting a GeoData
    const char* nodeType()                                      const override;

    /**
    * @brief this function returns the hotspot and the units the hotspot is measured in
    * @param xunits after running hotspot, you'll receive the unit in which x is measured
    * @param yunits the same for y
    * @return the point of the hotspot
    */
    const QPointF& hotSpot(Units& xunits, Units& yunits)        const;

    void setHotSpot(const QPointF& hotSpot = QPointF(0.5, 0.5),
                    Units xunits = Fraction, Units yunits = Fraction);

    /// Serialize the contents of the feature to @p stream.
    void pack(QDataStream& stream)                              const override;
    /// Unserialize the contents of the feature from @p stream.
    void unpack(QDataStream& stream)                                  override;

private:

    GeoDataHotSpotPrivate* const d = nullptr;
};

} // namespace Marble
