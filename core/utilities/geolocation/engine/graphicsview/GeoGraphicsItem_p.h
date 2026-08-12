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

#include <QSet>

// Local includes

#include "GeoGraphicsItem.h"
#include "GeoDataStyle.h"
#include "StyleBuilder.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoGraphicsItemPrivate
{
public:

    explicit GeoGraphicsItemPrivate(const GeoDataFeature* feature)
        : m_zValue      (0.0),
          m_minZoomLevel(0),
          m_feature     (feature),
          m_styleBuilder(nullptr),
          m_highlighted (false)
    {
    }

    virtual ~GeoGraphicsItemPrivate()
    {
    }

public:

    qreal                                   m_zValue              = 0.0;
    GeoGraphicsItem::GeoGraphicsItemFlags   m_flags;

    int                                     m_minZoomLevel        = 0;
    const GeoDataFeature*                   m_feature             = nullptr;
    RenderContext                           m_renderContext;
    GeoDataStyle::ConstPtr                  m_style;
    const StyleBuilder*                     m_styleBuilder        = nullptr;
    QVector<const GeoDataRelation*>         m_relations;

    QStringList                             m_paintLayers;

    // To highlight a placemark
    bool                                    m_highlighted         = false;
    GeoDataStyle::ConstPtr                  m_highlightStyle;
};

} // Namespace Marble
