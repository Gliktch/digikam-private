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
#include "GeoDataPoint.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoDataMultiTrack.h"
#include "GeoDataTrack.h"
#include "GeoDataTypes.h"
#include "GeoDataMultiGeometry.h"
#include "OsmPlacemarkData.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataPlacemarkExtendedData
{
public:

    GeoDataPlacemarkExtendedData()
        : m_area(-1.0),
          m_isBalloonVisible(false)
    {
        // nothing to do
    }

    GeoDataPlacemarkExtendedData& operator=(const GeoDataPlacemarkExtendedData& other)
    {
        m_countrycode = other.m_countrycode;
        m_area = other.m_area;
        m_state = other.m_state;
        m_isBalloonVisible = other.m_isBalloonVisible;
        return *this;
    }

    bool operator==(const GeoDataPlacemarkExtendedData& other) const
    {
        return m_countrycode == other.m_countrycode &&
               m_area == other.m_area &&
               m_state == other.m_state;
    }

    bool operator!=(const GeoDataPlacemarkExtendedData& other) const
    {
        return !(*this == other);
    }

    QString             m_countrycode;      // Country code.
    qreal               m_area;             // Area in square kilometer
    QString             m_state;            // State
    bool                m_isBalloonVisible; //Visibility of balloon
};

class Q_DECL_HIDDEN GeoDataPlacemarkPrivate : public GeoDataFeaturePrivate
{
    Q_DECLARE_TR_FUNCTIONS(GeoDataPlacemark)

public:

    GeoDataPlacemarkPrivate()
        : m_geometry(new GeoDataPoint),
          m_population(-1),
          m_placemarkExtendedData(nullptr),
          m_visualCategory(GeoDataPlacemark::Default),
          m_osmPlacemarkData(nullptr)
    {
    }

    GeoDataPlacemarkPrivate(const GeoDataPlacemarkPrivate& other)
        : GeoDataFeaturePrivate(other),
          m_geometry(other.m_geometry->copy()),
          m_population(other.m_population),
          m_placemarkExtendedData(nullptr),
          m_visualCategory(other.m_visualCategory),
          m_osmPlacemarkData(nullptr)
    {
        if (other.m_placemarkExtendedData)
        {
            m_placemarkExtendedData = new GeoDataPlacemarkExtendedData(*other.m_placemarkExtendedData);
        }

        if (other.m_osmPlacemarkData)
        {
            m_osmPlacemarkData = new OsmPlacemarkData(*other.m_osmPlacemarkData);
        }
    }

    ~GeoDataPlacemarkPrivate() override
    {
        delete m_geometry;
        delete m_placemarkExtendedData;
        delete m_osmPlacemarkData;
    }

    GeoDataPlacemarkPrivate& operator=(const GeoDataPlacemarkPrivate& other)
    {
        if (this == &other)
        {
            return *this;
        }

        GeoDataFeaturePrivate::operator=(other);

        m_population = other.m_population;
        m_visualCategory = other.m_visualCategory;

        delete m_geometry;
        m_geometry = other.m_geometry->copy();
        // TODO: why not set parent here to geometry?

        delete m_placemarkExtendedData;

        if (other.m_placemarkExtendedData)
        {
            m_placemarkExtendedData = new GeoDataPlacemarkExtendedData(*other.m_placemarkExtendedData);
        }

        else
        {
            m_placemarkExtendedData = nullptr;
        }

        delete m_osmPlacemarkData;

        if (other.m_osmPlacemarkData)
        {
            m_osmPlacemarkData = new OsmPlacemarkData(*other.m_osmPlacemarkData);
        }

        else
        {
            m_osmPlacemarkData = nullptr;
        }

        return *this;
    }

    EnumFeatureId featureId() const override
    {
        return GeoDataPlacemarkId;
    }

    GeoDataPlacemarkExtendedData& placemarkExtendedData()
    {
        if (!m_placemarkExtendedData)
        {
            m_placemarkExtendedData = new GeoDataPlacemarkExtendedData;
        }

        return *m_placemarkExtendedData;
    }

    const GeoDataPlacemarkExtendedData& placemarkExtendedData() const
    {
        return m_placemarkExtendedData ? *m_placemarkExtendedData : s_nullPlacemarkExtendedData;
    }

    OsmPlacemarkData& osmPlacemarkData()
    {
        if (!m_osmPlacemarkData)
        {
            m_osmPlacemarkData = new OsmPlacemarkData;
        }

        return *m_osmPlacemarkData;
    }

    const OsmPlacemarkData& osmPlacemarkData() const
    {
        return m_osmPlacemarkData ? *m_osmPlacemarkData : s_nullOsmPlacemarkData;
    }

public:

    static const OsmPlacemarkData             s_nullOsmPlacemarkData;
    static const GeoDataPlacemarkExtendedData s_nullPlacemarkExtendedData;

public:

    // Data for a Placemark in addition to those in GeoDataFeature.

    GeoDataGeometry*                        m_geometry              = nullptr;  // any GeoDataGeometry entry like locations
    qint64                                  m_population;                       // population in number of inhabitants
    GeoDataPlacemarkExtendedData*           m_placemarkExtendedData = nullptr;
    GeoDataPlacemark::GeoDataVisualCategory m_visualCategory;                   // the visual category

    OsmPlacemarkData*                       m_osmPlacemarkData      = nullptr;

};

} // namespace Marble
