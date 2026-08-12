/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-12-22
 * Description : a magnifier tool for the GraphicsDImgView canvas.
 *
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QGraphicsItem>
#include <QPixmap>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MagnifierItem : public QGraphicsItem
{

public:

    explicit MagnifierItem(QGraphicsItem* const parent = nullptr);
    ~MagnifierItem()                                                          override;

    void setSourcePixmap(const QPixmap& pixmap, const QRectF& rect);

    void setZoomFactor(qreal factor);
    qreal zoomFactor()                                                  const;

    void setMagnifierSize(int magnifierSize);
    int  magnifierSize()                                                const;

public:

    static QList<qreal> zoomFactors();

protected:

    QRectF boundingRect()                                               const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget)                                               override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
