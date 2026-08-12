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

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

class QEvent;
class QObject;
class QPainter;
class QRectF;
class QSizeF;
class QPointF;

namespace Marble
{

class AbstractMarbleGraphicsLayout;
class ViewportParams;

class MarbleGraphicsItemPrivate;

class DIGIKAM_EXPORT MarbleGraphicsItem
{
public:

    enum CacheMode
    {
        NoCache,
        ItemCoordinateCache,
        DeviceCoordinateCache
    };

public:

    virtual ~MarbleGraphicsItem();

    /**
     * Paints the item on the screen in view coordinates.
     * It is not safe to call this function from a thread other than the gui thread.
     */
    bool paintEvent(QPainter* painter, const ViewportParams* viewport);

    /**
     * Returns true if the Item contains @p point in parent coordinates.
     */
    bool contains(const QPointF& point)                         const;

    /**
     * Returns the layout of the MarbleGraphicsItem.
     */
    AbstractMarbleGraphicsLayout* layout()                      const;

    /**
     * Set the layout of the graphics item. The layout will now handle positions of added child
     * items. The MarbleGraphicsItem takes ownership of the layout.
     */
    void setLayout(AbstractMarbleGraphicsLayout* layout);

    /**
     * Returns the cache mode of the item
     */
    CacheMode cacheMode()                                       const;

    /**
     * Set the cache mode of the item
     */
    void setCacheMode(CacheMode mode);

    /**
     * Returns if the item is visible.
     */
    bool visible()                                              const;

    /**
     * Makes the item visible or invisible, depending on @p visible.
     */
    void setVisible(bool visible);

    /**
     * Hides the item. Equivalent to setVisible( false )
     */
    void hide();

    /**
     * Shows the item. Equivalent to setVisible( true )
     */
    void show();

    /**
     * Returns the size of the item
     */
    QSizeF size()                                               const;

    /**
     * Set the size of the item
     */
    void setSize(const QSizeF& size);

    /**
     * Returns the size of the content of the MarbleGraphicsItem.
     * This is identical to size() for default MarbleGraphicsItems.
     */
    virtual QSizeF contentSize()                                const;

    /**
     * Set the size of the content of the item.
     */
    virtual void setContentSize(const QSizeF& size);

    /**
     * Returns the rect of the content in item coordinates.
     */
    virtual QRectF contentRect()                                const;

    virtual void setProjection(const ViewportParams* viewport);

protected:

    explicit MarbleGraphicsItem(MarbleGraphicsItemPrivate* dd);

    /**
     * Paints the item in item coordinates. This has to be reimplemented by the subclass
     * This function will be called by paintEvent().
     */
    virtual void paint(QPainter* painter);

    virtual bool eventFilter(QObject* object, QEvent* e);

    /**
     * Marks the item and all parent items as invalid. If caching is enabled, the next paintEvent()
     * will cause the cache to be recreated, such that the paintEvent()s after will be optimized.
     */
    void update();

protected:

    MarbleGraphicsItemPrivate* const d_ptr = nullptr;

private:

    Q_DISABLE_COPY(MarbleGraphicsItem)
    Q_DECLARE_PRIVATE(MarbleGraphicsItem)
};

} // Namespace Marble
