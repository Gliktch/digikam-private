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

#include "FrameGraphicsItem.h"
#include "ScreenGraphicsItem_p.h"

// Qt includes

#include <QDebug>
#include <QBrush>

namespace Marble
{

class Q_DECL_HIDDEN FrameGraphicsItemPrivate : public ScreenGraphicsItemPrivate
{
public:

    FrameGraphicsItemPrivate(FrameGraphicsItem* frameGraphicsItem,
                             MarbleGraphicsItem* parent)
        : ScreenGraphicsItemPrivate(frameGraphicsItem, parent),
          m_frame                  (FrameGraphicsItem::NoFrame),
          m_contentSize            (0.0, 0.0),
          m_margin                 (0.0),
          m_marginTop              (0.0),
          m_marginBottom           (0.0),
          m_marginLeft             (0.0),
          m_marginRight            (0.0),
          m_padding                (0.0),
          m_borderWidth            (1.0),
          m_borderBrush            (QBrush(Qt::black)),
          m_borderStyle            (Qt::SolidLine),
          m_backgroundBrush        (QBrush(QColor(192, 192, 192, 192)))
    {
    }

    void updateSize()
    {
        qreal const border2 = 0.5 * m_borderWidth;
        qreal marginTop     = qMax(border2, (m_marginTop == 0.0)    ? m_margin : m_marginTop);
        qreal marginBottom  = qMax(border2, (m_marginBottom == 0.0) ? m_margin : m_marginBottom);
        qreal marginLeft    = qMax(border2, (m_marginLeft == 0.0)   ? m_margin : m_marginLeft);
        qreal marginRight   = qMax(border2, (m_marginRight == 0.0)  ? m_margin : m_marginRight);

        QSizeF totalSize    = m_contentSize;
        totalSize          += QSizeF(marginLeft + marginRight, marginTop + marginBottom);
        totalSize          += QSizeF(m_padding * 2, m_padding * 2);

        m_marbleGraphicsItem->setSize(totalSize);
    }

public:

    FrameGraphicsItem::FrameType    m_frame;
    QSizeF                          m_contentSize;

    // Margin

    qreal                           m_margin;
    qreal                           m_marginTop;
    qreal                           m_marginBottom;
    qreal                           m_marginLeft;
    qreal                           m_marginRight;

    // Padding

    qreal                           m_padding;

    // Background/Border painting

    qreal                           m_borderWidth;
    QBrush                          m_borderBrush;
    Qt::PenStyle                    m_borderStyle;
    QBrush                          m_backgroundBrush;
};

} // namespace Marble
