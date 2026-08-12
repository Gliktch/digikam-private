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

#include "GeoSceneDocument.h"

// Local includes

#include "GeoSceneHead.h"
#include "GeoSceneMap.h"
#include "GeoSceneLegend.h"
#include "GeoSceneSettings.h"
#include "GeoSceneTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoSceneDocumentPrivate
{
public:

    GeoSceneDocumentPrivate()
        : m_head    (new GeoSceneHead),
          m_map     (new GeoSceneMap),
          m_settings(new GeoSceneSettings),
          m_legend  (new GeoSceneLegend)
    {
    }

    ~GeoSceneDocumentPrivate()
    {
        delete m_head;
        delete m_map;
        delete m_settings;
        delete m_legend;
    }

public:

    GeoSceneHead*     m_head        = nullptr;
    GeoSceneMap*      m_map         = nullptr;
    GeoSceneSettings* m_settings    = nullptr;
    GeoSceneLegend*   m_legend      = nullptr;

private:

    GeoSceneDocumentPrivate(const GeoSceneDocumentPrivate&)            = delete;
    GeoSceneDocumentPrivate& operator=(const GeoSceneDocumentPrivate&) = delete;
};

GeoSceneDocument::GeoSceneDocument()
    : GeoDocument(),
      d          (new GeoSceneDocumentPrivate)
{
    // Establish connection of property changes to the outside, e.g. the LegendBrowser

    connect(d->m_settings, SIGNAL(valueChanged(QString,bool)),
            this, SIGNAL(valueChanged(QString,bool)));
}

GeoSceneDocument::~GeoSceneDocument()
{
    delete d;
}

const char* GeoSceneDocument::nodeType() const
{
    return GeoSceneTypes::GeoSceneDocumentType;
}

const GeoSceneHead* GeoSceneDocument::head() const
{
    return d->m_head;
}

GeoSceneHead* GeoSceneDocument::head()
{
    return d->m_head;
}

const GeoSceneMap* GeoSceneDocument::map() const
{
    return d->m_map;
}

GeoSceneMap* GeoSceneDocument::map()
{
    return d->m_map;
}

const GeoSceneSettings* GeoSceneDocument::settings() const
{
    return d->m_settings;
}

GeoSceneSettings* GeoSceneDocument::settings()
{
    return d->m_settings;
}

const GeoSceneLegend* GeoSceneDocument::legend() const
{
    return d->m_legend;
}

GeoSceneLegend* GeoSceneDocument::legend()
{
    return d->m_legend;
}

} // namespace Marble

#include "moc_GeoSceneDocument.cpp"
