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

#include "GeoDataStyle.h"
#include "digikam_export.h"

class QString;

namespace Marble
{

class GeoDataFeature;
class GeoDataLatLonAltBox;
class GeoDataCoordinates;
class GeoGraphicsItemPrivate;
class GeoPainter;
class StyleBuilder;
class ViewportParams;
class GeoDataRelation;

class RenderContext
{
public:

    bool operator==(const RenderContext& other) const;
    bool operator!=(const RenderContext& other) const;

    explicit RenderContext(int tileLevel = -1);
    int tileLevel() const;

private:

    int m_tileLevel = -1;
};

class DIGIKAM_EXPORT GeoGraphicsItem
{
public:

    enum GeoGraphicsItemFlag
    {
        NoOptions = 0x0,
        ItemIsMovable = 0x1,
        ItemIsSelectable = 0x2,
        ItemIsVisible = 0x4
    };
    Q_DECLARE_FLAGS(GeoGraphicsItemFlags, GeoGraphicsItemFlag)

public:

    explicit GeoGraphicsItem(const GeoDataFeature* feature);
    virtual ~GeoGraphicsItem();


    bool visible()                                                                          const;

    void setVisible(bool visible);

    /**
     * Get the GeoGraphicItemFlags value that describes which flags are set on
     * this item. @see QFlags
     */
    GeoGraphicsItemFlags flags()                                                            const;

    /**
     * Set or unset a single flag
     * @param flag the flag
     * @param enabled sets if the flag is to be set or unset
     */
    void setFlag(GeoGraphicsItemFlag flag, bool enabled = true);

    /**
     * Replace all of the current flags.
     * @param flags is the new value for this item's flags.
     */
    void setFlags(GeoGraphicsItemFlags flags);

    /**
     * Returns the minim zoom level on which item will be active.
     */
    int minZoomLevel()                                                                      const;

    /**
     * Sets the minimum zoom level
     */
    void setMinZoomLevel(int zoomLevel);

    /**
     * Returns the placemark for that item.
     */
    const GeoDataFeature* feature()                                                         const;

    /**
     * Returns the bounding box covered by the item.
     */
    virtual const GeoDataLatLonAltBox& latLonAltBox()                                       const = 0;

    /**
     * Returns the style of item.
     */
    GeoDataStyle::ConstPtr style()                                                          const;

    /**
     * Set the style for the item.
     */
    void setStyleBuilder(const StyleBuilder* styleBuilder);

    void resetStyle();

    /**
     * Set the style which will be used when
     * placemark is highlighted.
     * GeoGraphicsItem takes ownership of the
     * passed style and deletes it when appropriate.
     */
    void setHighlightStyle(const GeoDataStyle::ConstPtr& highlightStyle);

    /**
     * Returns the z value of the item
     */
    qreal zValue()                                                                          const;

    /**
     * Set the z value of the item
     */
    void setZValue(qreal z);

    static bool zValueLessThan(GeoGraphicsItem* one, GeoGraphicsItem* two);
    static bool styleLessThan(GeoGraphicsItem* one, GeoGraphicsItem* two);
    static bool zValueAndStyleLessThan(GeoGraphicsItem* one, GeoGraphicsItem* two);

    /**
     * Paints the item using the given GeoPainter.
     *
     * Note that depending on the projection and zoom level, the item may be visible more than once,
     * which is taken care of by GeoPainter.
     */
    virtual void paint(GeoPainter* painter,
                       const ViewportParams* viewport,
                       const QString& layer,
                       int tileZoomLevel) = 0;

    void setHighlighted(bool highlight);

    bool isHighlighted()                                                                    const;

    QStringList paintLayers()                                                               const;

    void setPaintLayers(const QStringList& paintLayers);

    void setRenderContext(const RenderContext& renderContext);

    /**
     * @brief Returns true if the item contains the given coordinates
     * @param screenPosition
     * @param viewport
     */
    virtual bool contains(const QPoint& screenPosition, const ViewportParams* viewport)     const;

    void setRelations(const QSet<const GeoDataRelation*>& relations);

protected:

    virtual void handleRelationUpdate(const QVector<const GeoDataRelation*>& relations);

protected:

    GeoGraphicsItemPrivate* const d = nullptr;

private:

    GeoGraphicsItem(const GeoGraphicsItem&)            = delete;
    GeoGraphicsItem& operator=(const GeoGraphicsItem&) = delete;
};

} // Namespace Marble

Q_DECLARE_OPERATORS_FOR_FLAGS(Marble::GeoGraphicsItem::GeoGraphicsItemFlags)
