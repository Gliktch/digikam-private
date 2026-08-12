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

#include <QAtomicInt>

// Local includes

#include "GeoDataGeometry.h"
#include "GeoDataLatLonAltBox.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataGeometryPrivate
{
public:

    GeoDataGeometryPrivate()
        : m_extrude(false),
          m_altitudeMode(ClampToGround),
          ref(0)
    {
    }

    GeoDataGeometryPrivate(const GeoDataGeometryPrivate& other)
        : m_extrude(other.m_extrude),
          m_altitudeMode(other.m_altitudeMode),
          m_latLonAltBox(),
          ref(0)
    {
    }

    virtual ~GeoDataGeometryPrivate()
    {
    }

    GeoDataGeometryPrivate& operator=(const GeoDataGeometryPrivate& other)
    {
        m_extrude      = other.m_extrude;
        m_altitudeMode = other.m_altitudeMode;
        m_latLonAltBox = other.m_latLonAltBox;

        return *this;
    }

    virtual GeoDataGeometryPrivate* copy() const = 0;

public:

    bool                        m_extrude;
    AltitudeMode                m_altitudeMode;
    mutable GeoDataLatLonAltBox m_latLonAltBox;

    QAtomicInt                  ref;
};

} // namespace Marble
