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

#include "GeoDataFeature.h"

// Qt includes

#include <QString>

// Local includes

#include "GeoDataExtendedData.h"
#include "GeoDataAbstractView.h"
#include "GeoDataRegion.h"
#include "GeoDataTimeStamp.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataTypes.h"
#include "GeoDataStyle.h"
#include "GeoDataSnippet.h"
#include "GeoDataLookAt.h"
#include "GeoDataCamera.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataFeatureExtendedData
{
public:

    GeoDataSnippet          m_snippet;                      ///< Snippet of the feature.
    QString                 m_description;                  ///< A longer textual description
    bool                    m_descriptionCDATA = false;     ///< True if description should be considered CDATA
    QString                 m_address;                      ///< The address.  Optional
    QString                 m_phoneNumber;                  ///< Phone         Optional
    GeoDataAbstractView*    m_abstractView     = nullptr;   ///< AbstractView  Optional
    GeoDataTimeSpan         m_timeSpan;
    GeoDataTimeStamp        m_timeStamp;
    GeoDataRegion           m_region;

public:

    GeoDataFeatureExtendedData()
        : m_snippet         (),
          m_description     (),
          m_descriptionCDATA(false),
          m_address         (),
          m_phoneNumber     (),
          m_abstractView    (nullptr),
          m_timeSpan        (),
          m_timeStamp       (),
          m_region          ()
    {
        // nothing to do
    }

    GeoDataFeatureExtendedData(const GeoDataFeatureExtendedData& other)
        : m_snippet         (other.m_snippet),
          m_description     (other.m_description),
          m_descriptionCDATA(other.m_descriptionCDATA),
          m_address         (other.m_address),
          m_phoneNumber     (other.m_phoneNumber),
          m_abstractView    (other.m_abstractView),
          m_timeSpan        (other.m_timeSpan),
          m_timeStamp       (other.m_timeStamp),
          m_region          (other.m_region)
    {
        // nothing to do
    }

    GeoDataFeatureExtendedData& operator=(const GeoDataFeatureExtendedData& other)
    {
        m_snippet           = other.m_snippet;
        m_description       = other.m_description;
        m_descriptionCDATA  = other.m_descriptionCDATA;
        m_address           = other.m_address;
        m_phoneNumber       = other.m_phoneNumber;
        m_abstractView      = other.m_abstractView;
        m_timeSpan          = other.m_timeSpan;
        m_timeStamp         = other.m_timeStamp;
        m_region            = other.m_region;

        return *this;
    }

    bool operator==(const GeoDataFeatureExtendedData& other) const
    {
        if (
            (m_snippet           != other.m_snippet)          ||
            (m_description       != other.m_description)      ||
            (m_descriptionCDATA  != other.m_descriptionCDATA) ||
            (m_address           != other.m_address)          ||
            (m_phoneNumber       != other.m_phoneNumber)      ||
            (m_timeSpan          != other.m_timeSpan)         ||
            (m_timeStamp         != other.m_timeStamp)        ||
            (m_region            != other.m_region)
           )
        {
            return false;
        }

        if (
            (!m_abstractView && other.m_abstractView) ||
            (m_abstractView && !other.m_abstractView)
           )
        {
            return false;
        }

        if (m_abstractView && other.m_abstractView)
        {
            if (*m_abstractView != *other.m_abstractView)
            {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const GeoDataFeatureExtendedData& other) const
    {
        return !(*this == other);
    }
};

class Q_DECL_HIDDEN GeoDataFeaturePrivate
{
public:

    GeoDataFeaturePrivate()
        : m_name                (),
          m_styleUrl            (),
          m_popularity          (0),
          m_zoomLevel           (1),
          m_visible             (true),
          m_role                (QLatin1String(" ")),
          m_style               (nullptr),
          m_styleMap            (nullptr),
          m_extendedData        (),
          m_featureExtendedData (nullptr)
    {
    }

    GeoDataFeaturePrivate(const GeoDataFeaturePrivate& other)
        : m_name                (other.m_name),
          m_styleUrl            (other.m_styleUrl),
          m_popularity          (other.m_popularity),
          m_zoomLevel           (other.m_zoomLevel),
          m_visible             (other.m_visible),
          m_role                (other.m_role),
          m_style               (other.m_style),                    // FIXME: both style and stylemap need to be reworked internally!!!!
          m_styleMap            (other.m_styleMap),
          m_extendedData        (other.m_extendedData),
          m_featureExtendedData (nullptr)
    {
        if (other.m_featureExtendedData)
        {
            m_featureExtendedData = new GeoDataFeatureExtendedData(*other.m_featureExtendedData);
        }
    }

    GeoDataFeaturePrivate& operator=(const GeoDataFeaturePrivate& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_name                = other.m_name;
        m_styleUrl            = other.m_styleUrl;
        m_popularity          = other.m_popularity;
        m_zoomLevel           = other.m_zoomLevel;
        m_visible             = other.m_visible;
        m_role                = other.m_role;
        m_style               = other.m_style;
        m_styleMap            = other.m_styleMap;
        m_extendedData        = other.m_extendedData;

        delete m_featureExtendedData;

        m_featureExtendedData = nullptr;

        if (other.m_featureExtendedData)
        {
            m_featureExtendedData = new GeoDataFeatureExtendedData(*other.m_featureExtendedData);
        }

        return *this;
    }

    virtual EnumFeatureId featureId() const
    {
        return InvalidFeatureId;
    }

    virtual ~GeoDataFeaturePrivate()
    {
        delete m_featureExtendedData;
    }

    GeoDataFeatureExtendedData& featureExtendedData()
    {
        if (!m_featureExtendedData)
        {
            m_featureExtendedData = new GeoDataFeatureExtendedData;
        }

        return *m_featureExtendedData;
    }

    const GeoDataFeatureExtendedData& featureExtendedData() const
    {
        if (!m_featureExtendedData)
        {
            m_featureExtendedData = new GeoDataFeatureExtendedData;
        }

        return *m_featureExtendedData;
    }

public:

    QString                             m_name;                         ///< Name of the feature. Is shown on screen
    QString                             m_styleUrl;                     ///< styleUrl     Url#tag to a document wide style
    qint64                              m_popularity          = 1;      ///< Population/Area/Altitude depending on placemark(!)
    int                                 m_zoomLevel           = 0;      ///< Zoom Level of the feature

    bool                                m_visible             = true;   ///< True if this feature should be shown.

    QString                             m_role;

    GeoDataStyle::Ptr                   m_style               = nullptr;
    const GeoDataStyleMap*              m_styleMap            = nullptr;

    GeoDataExtendedData                 m_extendedData;
    mutable GeoDataFeatureExtendedData* m_featureExtendedData = nullptr;
};

} // namespace Marble
