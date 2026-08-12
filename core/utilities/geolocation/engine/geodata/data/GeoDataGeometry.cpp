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

#include "GeoDataGeometry.h"
#include "GeoDataGeometry_p.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "GeoDataLinearRing.h"
#include "GeoDataLineString.h"
#include "GeoDataModel.h"
#include "GeoDataMultiGeometry.h"
#include "GeoDataMultiTrack.h"
#include "GeoDataPoint.h"
#include "GeoDataPolygon.h"
#include "GeoDataTrack.h"
#include "GeoDataTypes.h"
#include "digikam_debug.h"

namespace Marble
{

GeoDataGeometry::GeoDataGeometry(const GeoDataGeometry& other)
    : GeoDataObject(),
      d_ptr        (other.d_ptr)
{
    d_ptr->ref.ref();
}

GeoDataGeometry::GeoDataGeometry(GeoDataGeometryPrivate* priv)
    : GeoDataObject(),
      d_ptr        (priv)
{
    d_ptr->ref.ref();
}

GeoDataGeometry::~GeoDataGeometry()
{
    if (!d_ptr->ref.deref())
    {
        delete d_ptr;
    }
}

void GeoDataGeometry::detach()
{
    if (d_ptr->ref.fetchAndAddRelaxed(0) == 1)
    {
        return;
    }

    GeoDataGeometryPrivate* new_d = d_ptr->copy();

    if (!d_ptr->ref.deref())
    {
        delete d_ptr;
    }

    d_ptr = new_d;
    d_ptr->ref.ref();
}

GeoDataGeometry& GeoDataGeometry::operator=(const GeoDataGeometry& other)
{
    if (this == &other)
    {
        return *this;
    }

    GeoDataObject::operator=(other);

    if (!d_ptr->ref.deref())
    {
        delete d_ptr;
    }

    d_ptr = other.d_ptr;
    d_ptr->ref.ref();

    return *this;
}

bool GeoDataGeometry::operator==(const GeoDataGeometry& other) const
{
    if (nodeType() != other.nodeType())
    {
        return false;
    }

    if      (nodeType() == GeoDataTypes::GeoDataPolygonType)
    {
        const GeoDataPolygon& thisPoly  = static_cast<const GeoDataPolygon&>(*this);
        const GeoDataPolygon& otherPoly = static_cast<const GeoDataPolygon&>(other);

        return (thisPoly == otherPoly);
    }
    else if (nodeType() == GeoDataTypes::GeoDataLinearRingType)
    {
        const GeoDataLinearRing& thisRing  = static_cast<const GeoDataLinearRing&>(*this);
        const GeoDataLinearRing& otherRing = static_cast<const GeoDataLinearRing&>(other);

        return (thisRing == otherRing);
    }
    else if (nodeType() == GeoDataTypes::GeoDataLineStringType)
    {
        const GeoDataLineString& thisLine  = static_cast<const GeoDataLineString&>(*this);
        const GeoDataLineString& otherLine = static_cast<const GeoDataLineString&>(other);

        return (thisLine == otherLine);
    }
    else if (nodeType() == GeoDataTypes::GeoDataModelType)
    {
        const GeoDataModel& thisModel  = static_cast<const GeoDataModel&>(*this);
        const GeoDataModel& otherModel = static_cast<const GeoDataModel&>(other);

        return (thisModel == otherModel);
    }
    else if (nodeType() == GeoDataTypes::GeoDataMultiGeometryType)
    {
        const GeoDataMultiGeometry& thisMG  = static_cast<const GeoDataMultiGeometry&>(*this);
        const GeoDataMultiGeometry& otherMG = static_cast<const GeoDataMultiGeometry&>(other);

        return (thisMG == otherMG);
    }
    else if (nodeType() == GeoDataTypes::GeoDataTrackType)
    {
        const GeoDataTrack& thisTrack  = static_cast<const GeoDataTrack&>(*this);
        const GeoDataTrack& otherTrack = static_cast<const GeoDataTrack&>(other);

        return (thisTrack == otherTrack);
    }
    else if (nodeType() == GeoDataTypes::GeoDataMultiTrackType)
    {
        const GeoDataMultiTrack& thisMT  = static_cast<const GeoDataMultiTrack&>(*this);
        const GeoDataMultiTrack& otherMT = static_cast<const GeoDataMultiTrack&>(other);

        return (thisMT == otherMT);
    }
    else if (nodeType() == GeoDataTypes::GeoDataPointType)
    {
        const GeoDataPoint& thisPoint  = static_cast<const GeoDataPoint&>(*this);
        const GeoDataPoint& otherPoint = static_cast<const GeoDataPoint&>(other);

        return (thisPoint == otherPoint);
    }

    return false;
}

bool GeoDataGeometry::extrude() const
{
    return d_ptr->m_extrude;
}

void GeoDataGeometry::setExtrude(bool extrude)
{
    detach();
    d_ptr->m_extrude = extrude;
}

AltitudeMode GeoDataGeometry::altitudeMode() const
{
    return d_ptr->m_altitudeMode;
}

void GeoDataGeometry::setAltitudeMode(const AltitudeMode altitudeMode)
{
    detach();
    d_ptr->m_altitudeMode = altitudeMode;
}

const GeoDataLatLonAltBox& GeoDataGeometry::latLonAltBox() const
{
    return d_ptr->m_latLonAltBox;
}

void GeoDataGeometry::pack(QDataStream& stream) const
{
    GeoDataObject::pack(stream);

    stream << d_ptr->m_extrude;
    stream << d_ptr->m_altitudeMode;
}

void GeoDataGeometry::unpack(QDataStream& stream)
{
    detach();
    GeoDataObject::unpack(stream);

    int am;
    stream >> d_ptr->m_extrude;
    stream >> am;
    d_ptr->m_altitudeMode = (AltitudeMode) am;
}

bool GeoDataGeometry::equals(const GeoDataGeometry& other) const
{
    return (
            GeoDataObject::equals(other) &&
            (d_ptr->m_extrude      == other.d_ptr->m_extrude) &&
            (d_ptr->m_altitudeMode == other.d_ptr->m_altitudeMode)
           );
}

} // namespace Marble
