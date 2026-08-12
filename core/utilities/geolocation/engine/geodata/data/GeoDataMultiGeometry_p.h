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

#include "GeoDataGeometry_p.h"
#include "GeoDataPoint.h"
#include "GeoDataPolygon.h"
#include "GeoDataTrack.h"
#include "GeoDataMultiTrack.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"
#include "GeoDataModel.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataMultiGeometryPrivate : public GeoDataGeometryPrivate
{
public:

    GeoDataMultiGeometryPrivate()
    {
    }

    ~GeoDataMultiGeometryPrivate() override
    {
        qDeleteAll(m_vector);
    }

    GeoDataMultiGeometryPrivate& operator=(const GeoDataMultiGeometryPrivate& other)
    {
        GeoDataGeometryPrivate::operator=(other);

        qDeleteAll(m_vector);
        m_vector.clear();

        m_vector.reserve(other.m_vector.size());

        for (const GeoDataGeometry* geometry : other.m_vector)
        {

            m_vector.append(geometry->copy());
        }

        return *this;
    }

    GeoDataGeometryPrivate* copy() const override
    {
        GeoDataMultiGeometryPrivate* copy = new GeoDataMultiGeometryPrivate;
        *copy = *this;
        return copy;
    }

public:

    QVector<GeoDataGeometry*> m_vector;
};

} // namespace Marble
