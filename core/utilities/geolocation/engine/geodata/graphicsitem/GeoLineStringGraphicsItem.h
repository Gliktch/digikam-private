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

#include <QRegion>

// Local includes

#include "GeoGraphicsItem.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLineString.h"
#include "MarbleGlobal.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataPlacemark;

class DIGIKAM_EXPORT GeoLineStringGraphicsItem : public GeoGraphicsItem
{
    Q_DECLARE_TR_FUNCTIONS(GeoLineStringGraphicsItem)

public:

    explicit GeoLineStringGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataLineString* lineString);
    ~GeoLineStringGraphicsItem() override;

    void setLineString(const GeoDataLineString* lineString);
    const GeoDataLineString* lineString() const;
    static GeoDataLineString merge(const QVector<const GeoDataLineString*>& lineStrings);
    void setMergedLineString(const GeoDataLineString& sharedLineString);

    const GeoDataLatLonAltBox& latLonAltBox() const override;

    void paint(GeoPainter* painter, const ViewportParams* viewport, const QString& layer, int tileZoomLevel) override;
    bool contains(const QPoint& screenPosition, const ViewportParams* viewport) const override;

    static const GeoDataStyle* s_previousStyle;
    static bool s_paintInline;
    static bool s_paintOutline;

protected:

    void handleRelationUpdate(const QVector<const GeoDataRelation*>& relations) override;

private:

    void paintOutline(GeoPainter* painter, const ViewportParams* viewport) const;
    void paintInline(GeoPainter* painter, const ViewportParams* viewport);
    void paintLabel(GeoPainter* painter, const ViewportParams* viewport) const;

    bool configurePainterForLine(GeoPainter* painter, const ViewportParams* viewport, const bool isOutline = false) const;
    bool configurePainterForLabel(GeoPainter* painter,  const ViewportParams* viewport, LabelPositionFlags& labelPositionFlags) const;

    static bool canMerge(const GeoDataCoordinates& a, const GeoDataCoordinates& b);

private:

    const GeoDataLineString* m_lineString           = nullptr;
    const GeoDataLineString* m_renderLineString     = nullptr;
    GeoDataLineString        m_mergedLineString;
    QVector<QPolygonF*>      m_cachedPolygons;
    bool                     m_renderLabel;
    qreal                    m_penWidth;
    mutable QRegion          m_cachedRegion;
    QString                  m_name;
};

} // namespace Marble
