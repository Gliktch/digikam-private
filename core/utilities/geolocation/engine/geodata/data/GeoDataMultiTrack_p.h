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
#include "GeoDataTrack.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataMultiTrackPrivate : public GeoDataGeometryPrivate
{
public:

    GeoDataMultiTrackPrivate() = default;

    ~GeoDataMultiTrackPrivate() override
    {
        qDeleteAll(m_vector);
    }

    GeoDataMultiTrackPrivate& operator=(const GeoDataMultiTrackPrivate& other)
    {
        GeoDataGeometryPrivate::operator=(other);

        qDeleteAll(m_vector);
        m_vector.clear();

        m_vector.reserve(other.m_vector.size());

        for (const GeoDataTrack* const track : other.m_vector)
        {
            m_vector.append(new GeoDataTrack(*track));
        }

        return *this;
    }

    GeoDataGeometryPrivate* copy() const override
    {
        GeoDataMultiTrackPrivate* const copy = new GeoDataMultiTrackPrivate;
        *copy = *this;

        return copy;
    }

public:

    QVector<GeoDataTrack*> m_vector;
};

} // namespace Marble
