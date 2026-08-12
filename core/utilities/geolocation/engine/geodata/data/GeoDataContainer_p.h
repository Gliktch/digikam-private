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

#include "GeoDataFeature_p.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataContainerPrivate : public GeoDataFeaturePrivate
{
public:

    GeoDataContainerPrivate() = default;

    GeoDataContainerPrivate(const GeoDataContainerPrivate& other)
        : GeoDataFeaturePrivate(other)
    {
        m_vector.reserve(other.m_vector.size());

        for (const GeoDataFeature* const feature : other.m_vector)
        {
            m_vector.append(feature->clone());
        }
    }

    ~GeoDataContainerPrivate() override
    {
        qDeleteAll(m_vector);
    }

    GeoDataContainerPrivate& operator=(const GeoDataContainerPrivate& other)
    {
        GeoDataFeaturePrivate::operator=(other);
        qDeleteAll(m_vector);
        m_vector.clear();
        m_vector.reserve(other.m_vector.size());

        for (const GeoDataFeature* const feature : other.m_vector)
        {
            m_vector.append(feature->clone());
        }

        return *this;
    }

    EnumFeatureId featureId() const override
    {
        return GeoDataFolderId;
    }

    void setParent(GeoDataObject* const parent)
    {
        for (GeoDataFeature* const feature : m_vector)
        {
            feature->setParent(parent);
        }
    }

public:

    QVector<GeoDataFeature*> m_vector;
};

} // namespace Marble
