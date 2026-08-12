/* @s============================================================
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

#include<QDebug>
#include<QList>
#include<QSet>
#include<QSize>
#include<QSizeF>
#include<QRect>
#include<QPixmap>

// Local includes

#include "AbstractMarbleGraphicsLayout.h"
#include "MarbleGraphicsItem.h"

namespace Marble
{

class Q_DECL_HIDDEN MarbleGraphicsItemPrivate
{
public:

    explicit MarbleGraphicsItemPrivate(MarbleGraphicsItem* marbleGraphicsItem,
                                       MarbleGraphicsItem* parent = nullptr)
        : m_repaintNeeded     (true),
          m_cacheMode         (MarbleGraphicsItem::NoCache),
          m_visibility        (true),
          m_parent            (parent),
          m_children          (),
          m_layout            (nullptr),
          m_marbleGraphicsItem(marbleGraphicsItem)
    {
        if (m_parent)
        {
            m_parent->d_func()->addChild(m_marbleGraphicsItem);
        }
    }

    virtual ~MarbleGraphicsItemPrivate()
    {
        // Remove from parent

        if (m_parent)
        {
            m_parent->d_func()->removeChild(m_marbleGraphicsItem);
        }

        // Delete all children using a copy, since children may invalidate m_children's iterator

        qDeleteAll(m_children.values());

        // Delete Layout

        delete m_layout;
    }

    void addChild(MarbleGraphicsItem* child)
    {
        m_children.insert(child);
    }

    void removeChild(MarbleGraphicsItem* child)
    {
        m_children.remove(child);
    }

    virtual QVector<QPointF> positions() const = 0;

    virtual QVector<QPointF> absolutePositions() const = 0;

    /**
     * @brief Used to get the set of screen bounding rects
     */
    QVector<QRectF> boundingRects() const;

    virtual void setProjection(const ViewportParams* viewport) = 0;

    void updateChildPositions()
    {
        // This has to be done recursively because we need a correct size from all children.

        for (MarbleGraphicsItem* item : m_children)
        {
            item->d_func()->updateChildPositions();
        }

        // Adjust positions

        if (m_layout)
        {
            m_layout->updatePositions(m_marbleGraphicsItem);
        }
    }

public:

    QSizeF                          m_size;

    bool                            m_repaintNeeded       = true;

    MarbleGraphicsItem::CacheMode   m_cacheMode           = MarbleGraphicsItem::NoCache;

    QPixmap                         m_pixmap;

    bool                            m_visibility          = true;

    // The parent of the item

    MarbleGraphicsItem* const       m_parent              = nullptr;

    // The set of children.

    QSet<MarbleGraphicsItem*>       m_children;

    // The layout handling the positions of the children

    AbstractMarbleGraphicsLayout*   m_layout              = nullptr;

    MarbleGraphicsItem* const       m_marbleGraphicsItem  = nullptr;

private:

    MarbleGraphicsItemPrivate(const MarbleGraphicsItemPrivate&)            = delete;
    MarbleGraphicsItemPrivate& operator=(const MarbleGraphicsItemPrivate&) = delete;
};

} // Namespace Marble
