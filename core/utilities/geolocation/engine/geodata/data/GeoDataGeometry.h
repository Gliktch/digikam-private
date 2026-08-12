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
#include "MarbleGlobal.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataGeometryPrivate;

class GeoDataLatLonAltBox;
class GeoDataLineString; // LinearRing is the same!
class GeoDataMultiGeometry;

/**
 * @short A base class for all geodata features
 *
 * GeoDataGeometry is the base class for most geodata classes that
 * deal with geometric data (points, linestrings, polygons, etc.)
 *
 * @see GeoDataLineString
 * @see GeoDataLinearRing
 * @see GeoDataPolygon
 */

class DIGIKAM_EXPORT GeoDataGeometry : public GeoDataObject
{
public:

    ~GeoDataGeometry() override;

    virtual EnumGeometryId geometryId() const = 0;

    virtual GeoDataGeometry* copy() const = 0;

    bool operator==(const GeoDataGeometry& other) const;
    inline bool operator!=(const GeoDataGeometry& other) const
    {
        return !(*this == other);
    }

    bool extrude() const;
    void setExtrude(bool extrude);

    AltitudeMode altitudeMode() const;
    void setAltitudeMode(const AltitudeMode altitudeMode);

    virtual const GeoDataLatLonAltBox& latLonAltBox() const;

    /// Serialize the contents of the feature to @p stream.
    void pack(QDataStream& stream) const override;
    /// Unserialize the contents of the feature from @p stream.
    void unpack(QDataStream& stream) override;

    void detach();

protected:

    explicit GeoDataGeometry(GeoDataGeometryPrivate* priv);
    explicit GeoDataGeometry(const GeoDataGeometry& other);
    GeoDataGeometry& operator=(const GeoDataGeometry& other);

    bool equals(const GeoDataGeometry& other) const;

    using GeoDataObject::equals;

protected:

    GeoDataGeometryPrivate* d_ptr = nullptr;
};

} // namespace Marble

Q_DECLARE_METATYPE(Marble::GeoDataGeometry*)
