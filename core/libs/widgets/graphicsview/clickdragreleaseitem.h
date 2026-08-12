/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-04
 * Description : A simple item to click, drag and release
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QGraphicsObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ClickDragReleaseItem : public QGraphicsObject      // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public:

    explicit ClickDragReleaseItem(QGraphicsItem* const parent);
    ~ClickDragReleaseItem()                                          override;

    QRectF boundingRect() const                                      override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

Q_SIGNALS:

    /**
     * Signals are emitted at click, drag and release event.
     * Reported positions are in scene coordinates.
     * A drag is reported only if the mouse was moved a certain threshold.
     * A release is reported after every press.
     */

    void started(const QPointF& pos);
    void moving(const QRectF& rect);
    void finished(const QRectF& rect);
    void cancelled();

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent*)                  override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*)                   override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*)                override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)            override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent*)                   override;
    void keyPressEvent(QKeyEvent*)                                   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
