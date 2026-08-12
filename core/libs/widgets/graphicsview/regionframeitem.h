/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-09
 * Description : tag region frame
 *
 * SPDX-FileCopyrightText: 2007      by Aurelien Gateau <agateau at kde dot org>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFlags>

// Local includes

#include "dimgchilditem.h"
#include "digikam_export.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT RegionFrameItem : public DImgChildItem         // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public:

    enum Flag
    {
        NoFlags           = 0,
        ShowResizeHandles = 1 << 0,
        MoveByDrag        = 1 << 1,

        GeometryEditable  = ShowResizeHandles | MoveByDrag
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:

    explicit RegionFrameItem(QGraphicsItem* const parent);
    ~RegionFrameItem()                                      override;

    void  setFlags(Flags flags);
    void  changeFlags(Flags flags, bool addOrRemove);
    Flags flags()                                     const;

    /**
     * Sets a widget item as HUD item. A HUD item will be positioned relative to this item,
     * and repositioned on position changes or resizing.
     * Ownership of the item is taken, and it is made a child item of this item.
     * You can also add QWidget directly. It will be wrapped in a proxy item.
     */
    void             setHudWidget(QGraphicsWidget* const hudWidget);
    void             setHudWidget(QWidget* const widget, Qt::WindowFlags wFlags = Qt::WindowFlags());
    QGraphicsWidget* hudWidget()                      const;
    void             setHudWidgetVisible(bool visible);

    void setFixedRatio(double ratio);

    void setRectInSceneCoordinatesAdjusted(const QRectF& rect);

public Q_SLOTS:

    /**
     * The associated HUD item is dynamically moved to be visible.
     * This can only be done for _one_ region at a time.
     * Set the current primary view region of the scene by this method
     * to dynamically reposition the HUD inside this region.
     * The rect given is in scene coordinates.
     */
    void setViewportRect(const QRectF& rect);

Q_SIGNALS:

    void geometryEdited();

protected:

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr)                   override;
    void mousePressEvent(QGraphicsSceneMouseEvent*)         override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*)          override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*)       override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event)   override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event)    override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event)   override;

    bool eventFilter(QObject* watched, QEvent* event)       override;

private Q_SLOTS:

    void slotUpdate();
    void slotSizeChanged();
    void slotPosChanged();
    void hudSizeChanged();
    void moveHudWidget();

private:

    /// @note disabled
    RegionFrameItem();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::RegionFrameItem::Flags)
