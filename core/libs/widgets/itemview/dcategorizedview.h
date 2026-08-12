/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Item view for listing items in a categorized fashion optionally
 *
 * SPDX-FileCopyrightText: 2007      by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QListView>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DCategoryDrawer;

/**
 * @short Item view for listing items
 *
 * DCategorizedView allows you to use it as it were a QListView.
 * Subclass DCategorizedSortFilterProxyModel to provide category information for items.
 */
class DIGIKAM_EXPORT DCategorizedView : public QListView
{
    Q_OBJECT

public:

    explicit DCategorizedView(QWidget* const parent = nullptr);
    ~DCategorizedView() override;

    void setGridSize(const QSize& size);

    void setCategoryDrawer(DCategoryDrawer* categoryDrawer);
    DCategoryDrawer* categoryDrawer()                                                   const;

    /**
     * Switch on drawing of dragged items. Default: on.
     * While dragging over the view, dragged items will be drawn transparently
     * following the mouse cursor.
     *
     * @param drawDraggedItems if <code>true</code>, dragged items will be
     *                         drawn
     */
    void setDrawDraggedItems(bool drawDraggedItems);

    void        setModel(QAbstractItemModel* model)                                    override;
    QRect       visualRect(const QModelIndex& index)                            const  override;
    QModelIndex indexAt(const QPoint& point)                                    const  override;

    /**
     * This method will return all indexes whose visual rect intersects @p rect.
     * @param rect rectangle to test intersection with
     * @note Returns an empty list if the view is not categorized.
     */
    virtual QModelIndexList categorizedIndexesIn(const QRect& rect)                     const;

    /**
     * This method will return the visual rect of the header of the category
     * in which @p index is sorted.
     * @note Returns QRect() if the view is not categorized.
     */
    virtual QRect categoryVisualRect(const QModelIndex& index)                          const;

    /**
     * This method will return the first index of the category
     * in the region of which @p point is found.
     * @note Returns QModelIndex() if the view is not categorized.
     */
    virtual QModelIndex categoryAt(const QPoint& point)                                 const;

    /**
     * This method returns the range of indexes contained
     * in the category in which @p index is sorted.
     * @note Returns an empty range if the view is no categorized.
     */
    virtual QItemSelectionRange categoryRange(const QModelIndex& index)                 const;

public Q_SLOTS:

    void reset()                                                                       override;

protected:

    void paintEvent(QPaintEvent* event)                                                override;

    void resizeEvent(QResizeEvent* event)                                              override;

    void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags flags)    override;

    void mouseMoveEvent(QMouseEvent* event)                                            override;

    void mousePressEvent(QMouseEvent* event)                                           override;

    void mouseReleaseEvent(QMouseEvent* event)                                         override;

    void leaveEvent(QEvent* event)                                                     override;

    void startDrag(Qt::DropActions supportedActions)                                   override;

    void dragMoveEvent(QDragMoveEvent* event)                                          override;

    void dragLeaveEvent(QDragLeaveEvent* event)                                        override;

    void dropEvent(QDropEvent* event)                                                  override;

    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

protected Q_SLOTS:

    void rowsInserted(const QModelIndex& parent, int start, int end)                   override;

    virtual void rowsInsertedArtifficial(const QModelIndex& parent, int start, int end);

    virtual void rowsRemoved(const QModelIndex& parent, int start, int end);

    void updateGeometries()                                                            override;

    virtual void slotLayoutChanged();

    void currentChanged(const QModelIndex& current, const QModelIndex& previous)       override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
