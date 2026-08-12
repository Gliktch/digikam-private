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

#include "dcategorizedview_p.h"

namespace Digikam
{

DCategorizedView::DCategorizedView(QWidget* const parent)
    : QListView(parent),
      d        (new Private(this))
{
}

DCategorizedView::~DCategorizedView()
{
    delete d;
}

void DCategorizedView::setGridSize(const QSize& size)
{
    QListView::setGridSize(size);

    slotLayoutChanged();
}

void DCategorizedView::setModel(QAbstractItemModel* model)
{
    d->forcedSelectionPosition = 0;
    d->hovered                 = QModelIndex();
    d->mouseButtonPressed      = false;
    d->rightMouseButtonPressed = false;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->lastSelection.clear();
    d->lastCategorySelection.clear();

    if (d->proxyModel)
    {
        QObject::disconnect(d->proxyModel, SIGNAL(layoutChanged()),
                            this, SLOT(slotLayoutChanged()));

        QObject::disconnect(d->proxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                            this, SLOT(rowsRemoved(QModelIndex,int,int)));
    }

    QListView::setModel(model);

    d->proxyModel = dynamic_cast<DCategorizedSortFilterProxyModel*>(model);

    if (d->proxyModel)
    {
        QObject::connect(d->proxyModel, SIGNAL(layoutChanged()),
                         this, SLOT(slotLayoutChanged()));

        QObject::connect(d->proxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                         this, SLOT(rowsRemoved(QModelIndex,int,int)));

        if (d->proxyModel->rowCount())
        {
            slotLayoutChanged();
        }
    }
}

QRect DCategorizedView::visualRect(const QModelIndex& index) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QListView::visualRect(index);
    }

    if (!qobject_cast<const QSortFilterProxyModel*>(index.model()))
    {
        return d->visualRect(d->proxyModel->mapFromSource(index));
    }

    return d->visualRect(index);
}

QRect DCategorizedView::categoryVisualRect(const QModelIndex& index) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QRect();
    }

    if (!index.isValid())
    {
        return QRect();
    }

    QString category = d->elementsInfo[index.row()].category;

    return d->categoryVisualRect(category);
}

QModelIndex DCategorizedView::categoryAt(const QPoint& point) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QModelIndex();
    }

    // We traverse the categories and find the first where point.y() is below the visualRect

    int     y = 0;
    int lastY = 0;
    QString lastCategory;

    for (const QString& category : std::as_const(d->categories))
    {
        y = d->categoryVisualRect(category).top();

        if ((point.y() >= lastY) && (point.y() < y))
        {
            break;
        }

        lastY        = y;
        y            = 0;
        lastCategory = category;
    }

    // if lastCategory is the last one in the list y will be 0

    if (!lastCategory.isNull() && (point.y() >= lastY) && ((point.y() < y) || !y))
    {
        return d->proxyModel->index(d->categoriesIndexes[lastCategory][0], d->proxyModel->sortColumn());
    }

    return QModelIndex();
}

QItemSelectionRange DCategorizedView::categoryRange(const QModelIndex& index) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QItemSelectionRange();
    }

    if (!index.isValid())
    {
        return QItemSelectionRange();
    }

    QString category  = d->elementsInfo[index.row()].category;
    QModelIndex first = d->proxyModel->index(d->categoriesIndexes[category].first(), d->proxyModel->sortColumn());
    QModelIndex last  = d->proxyModel->index(d->categoriesIndexes[category].last(), d->proxyModel->sortColumn());

    return QItemSelectionRange(first, last);
}

DCategoryDrawer* DCategorizedView::categoryDrawer() const
{
    return d->categoryDrawer;
}

void DCategorizedView::setCategoryDrawer(DCategoryDrawer* categoryDrawer)
{
    d->forcedSelectionPosition = 0;
    d->hovered                 = QModelIndex();
    d->mouseButtonPressed      = false;
    d->rightMouseButtonPressed = false;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->lastSelection.clear();
    d->lastCategorySelection.clear();
    d->categoryDrawer          = categoryDrawer;

    if (categoryDrawer)
    {
        if (d->proxyModel)
        {
            if (d->proxyModel->rowCount())
            {
                slotLayoutChanged();
            }
        }
    }
    else
    {
        updateGeometries();
    }
}

void DCategorizedView::setDrawDraggedItems(bool drawDraggedItems)
{
    d->drawItemsWhileDragging = drawDraggedItems;
}

QModelIndex DCategorizedView::indexAt(const QPoint& point) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QListView::indexAt(point);
    }

    QModelIndex index;

    const QModelIndexList item = d->intersectionSet(QRect(point, point));

    if (item.count() == 1)
    {
        index = item[0];
    }

    return index;
}

QModelIndexList DCategorizedView::categorizedIndexesIn(const QRect& rect) const
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return QModelIndexList();
    }

    return d->intersectionSet(rect);
}

void DCategorizedView::reset()
{
    QListView::reset();

    d->forcedSelectionPosition = 0;
    d->hovered                 = QModelIndex();
    d->biggestItemSize         = QSize(0, 0);
    d->mouseButtonPressed      = false;
    d->rightMouseButtonPressed = false;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();
    d->lastSelection.clear();
    d->lastCategorySelection.clear();
}

void DCategorizedView::paintEvent(QPaintEvent* event)
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::paintEvent(event);
        return;
    }

    bool alternatingRows          = alternatingRowColors();
    QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    initViewItemOption(&option);

#else

    option = viewOptions();

#endif

    option.widget                 = this;

    if (wordWrap())
    {
        option.features |= QStyleOptionViewItem::WrapText;
    }

    QPainter painter(viewport());
    QRect area                = event->rect();
    const bool focus          = (hasFocus() || viewport()->hasFocus()) && currentIndex().isValid();
    const QStyle::State state = option.state;
    const bool enabled        = ((state & QStyle::State_Enabled) != 0);

    painter.save();

    QModelIndexList dirtyIndexes = d->intersectionSet(area);
    bool alternate               = false;

    if (dirtyIndexes.count())
    {
        alternate = (dirtyIndexes[0].row() % 2);
    }

    for (const QModelIndex& index : std::as_const(dirtyIndexes))
    {
        if      (alternatingRows && alternate)
        {
            option.features |= QStyleOptionViewItem::Alternate;
            alternate        = false;
        }
        else if (alternatingRows)
        {
            option.features &= ~QStyleOptionViewItem::Alternate;
            alternate        = true;
        }

        option.state = state;
        option.rect  = visualRect(index);

        if (selectionModel() && selectionModel()->isSelected(index))
        {
            option.state |= QStyle::State_Selected;
        }

        if (enabled)
        {
            QPalette::ColorGroup cg;

            if ((d->proxyModel->flags(index) & Qt::ItemIsEnabled) == 0)
            {
                option.state &= ~QStyle::State_Enabled;
                cg            = QPalette::Disabled;
            }
            else
            {
                cg = QPalette::Normal;
            }

            option.palette.setCurrentColorGroup(cg);
        }

        if (focus && (currentIndex() == index))
        {
            option.state |= QStyle::State_HasFocus;

            if (this->state() == EditingState)
            {
                /**
                 * @note QStyle::State_Editing is deprecated: Not used and no effect since Qt 5.
                 */
                //option.state |= QStyle::State_Editing;
            }
        }

        if (index == d->hovered)
        {
            option.state |= QStyle::State_MouseOver;
        }
        else
        {
            option.state &= ~QStyle::State_MouseOver;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        itemDelegateForIndex(index)->paint(&painter, option, index);

#else

        itemDelegate(index)->paint(&painter, option, index);

#endif

    }

    // Redraw categories

    QStyleOptionViewItem otherOption;
    bool                 intersectedInThePast = false;

    for (const QString& category : std::as_const(d->categories))
    {
        otherOption        = option;
        otherOption.rect   = d->categoryVisualRect(category);
        otherOption.state &= ~QStyle::State_MouseOver;

        if      (otherOption.rect.intersects(area))
        {
            intersectedInThePast    = true;
            QModelIndex indexToDraw = d->proxyModel->index(d->categoriesIndexes[category][0],
                                                           d->proxyModel->sortColumn());

            d->drawNewCategory(indexToDraw, d->proxyModel->sortRole(), otherOption, &painter);
        }
        else if (intersectedInThePast)  // cppcheck-suppress knownConditionTrueFalse
        {
            // the visible area has been finished, we don't need to keep asking, the rest won't intersect
            // this is doable because we know that categories are correctly ordered on the list.

            break;
        }
    }

    if ((selectionMode() != SingleSelection) && (selectionMode() != NoSelection))
    {
        if (d->mouseButtonPressed && (QListView::state() != DraggingState))
        {
            QPoint start, end, initialPressPosition;

            initialPressPosition = d->initialPressPosition;

            initialPressPosition.setY(initialPressPosition.y() - verticalOffset());
            initialPressPosition.setX(initialPressPosition.x() - horizontalOffset());

            if (
                (d->initialPressPosition.x() > d->mousePosition.x()) ||
                (d->initialPressPosition.y() > d->mousePosition.y())
               )
            {
                start = d->mousePosition;
                end   = initialPressPosition;
            }
            else
            {
                start = initialPressPosition;
                end   = d->mousePosition;
            }

            QStyleOptionRubberBand yetAnotherOption;
            yetAnotherOption.initFrom(this);
            yetAnotherOption.shape  = QRubberBand::Rectangle;
            yetAnotherOption.opaque = false;
            yetAnotherOption.rect   = QRect(start, end).intersected(viewport()->rect().adjusted(-16, -16, 16, 16));
            painter.save();
            style()->drawControl(QStyle::CE_RubberBand, &yetAnotherOption, &painter);
            painter.restore();
        }
    }

    if (d->drawItemsWhileDragging && (QListView::state() == DraggingState) && !d->dragLeftViewport)
    {
        painter.setOpacity(0.5);
        d->drawDraggedItems(&painter);
    }

    painter.restore();
}

void DCategorizedView::resizeEvent(QResizeEvent* event)
{
    QListView::resizeEvent(event);

    // Clear the items positions cache

    d->elementsPosition.clear();
    d->categoriesPosition.clear();
    d->forcedSelectionPosition = 0;

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

    d->updateScrollbars();
}

QItemSelection DCategorizedView::Private::selectionForRect(const QRect& rect)
{
    QItemSelection selection;
    QModelIndex    tl, br;
    QModelIndexList intersectedIndexes    = intersectionSet(rect);
    QList<QModelIndex>::const_iterator it = intersectedIndexes.constBegin();

    for ( ; it != intersectedIndexes.constEnd() ; ++it)
    {
        if      (!tl.isValid() && !br.isValid())
        {
            tl = br = *it;
        }
        else if ((*it).row() == (tl.row() - 1))
        {
            tl = *it;   // expand current range
        }
        else if ((*it).row() == (br.row() + 1))
        {
            br = (*it); // expand current range
        }
        else
        {
            selection.select(tl, br); // select current range
            tl = br = *it;            // start new range
        }
    }

    if      (tl.isValid() && br.isValid())
    {
        selection.select(tl, br);
    }
    else if (tl.isValid())
    {
        selection.select(tl, tl);
    }
    else if (br.isValid())
    {
        selection.select(br, br);
    }

    return selection;
}

void DCategorizedView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::setSelection(rect, command);
        return;
    }

    QItemSelection selection;
/*
    QRect contentsRect = rect.translated(horizontalOffset(), verticalOffset());
*/
    QModelIndexList intersectedIndexes = d->intersectionSet(rect);

    if ((rect.width() == 1) && (rect.height() == 1))
    {
        QModelIndex tl;

        if (!intersectedIndexes.isEmpty())
        {
            tl = intersectedIndexes.last();    // special case for mouse press; only select the top item
        }

        if (tl.isValid() && (tl.flags() & Qt::ItemIsEnabled))
        {
            selection.select(tl, tl);
        }
    }
    else
    {
        if (state() == DragSelectingState)
        {
            // visual selection mode (rubberband selection)

            selection = d->selectionForRect(rect);
        }
        else
        {
            // logical selection mode (key and mouse click selection)

            QModelIndex tl, br;

            // get the first item

            const QRect topLeft(rect.left(), rect.top(), 1, 1);
            intersectedIndexes = d->intersectionSet(topLeft);

            if (!intersectedIndexes.isEmpty())
            {
                tl = intersectedIndexes.last();
            }

            // get the last item

            const QRect bottomRight(rect.right(), rect.bottom(), 1, 1);
            intersectedIndexes = d->intersectionSet(bottomRight);

            if (!intersectedIndexes.isEmpty())
            {
                br = intersectedIndexes.last();
            }

            // get the ranges

            if (
                tl.isValid()                     &&
                br.isValid()                     &&
                (tl.flags() & Qt::ItemIsEnabled) &&
                (br.flags() & Qt::ItemIsEnabled)
               )
            {
                // first, middle, last in content coordinates

                QRect middle;
                QRect first    = d->cachedRectIndex(tl);
                QRect last     = d->cachedRectIndex(br);
                QSize fullSize = d->contentsSize();

                if (flow() == LeftToRight)
                {
                    QRect& top    = first;
                    QRect& bottom = last;

                    // if bottom is above top, swap them

                    if (top.center().y() > bottom.center().y())
                    {
                        QRect tmp = top;
                        top       = bottom;
                        bottom    = tmp;
                    }

                    // if the rect are on different lines, expand

                    if      (top.top() != bottom.top())
                    {
                        // top rectangle

                        if (isRightToLeft())
                        {
                            top.setLeft(0);
                        }
                        else
                        {
                            top.setRight(fullSize.width());
                        }

                        // bottom rectangle

                        if (isRightToLeft())
                        {
                            bottom.setRight(fullSize.width());
                        }
                        else
                        {
                            bottom.setLeft(0);
                        }
                    }
                    else if (top.left() > bottom.right())
                    {
                        if (isRightToLeft())
                        {
                            bottom.setLeft(top.right());
                        }
                        else
                        {
                            bottom.setRight(top.left());
                        }
                    }
                    else
                    {
                        if (isRightToLeft())
                        {
                            top.setLeft(bottom.right());
                        }
                        else
                        {
                            top.setRight(bottom.left());
                        }
                    }

                    // middle rectangle

                    if (top.bottom() < bottom.top())
                    {
                        middle.setTop(top.bottom() + 1);
                        middle.setLeft(qMin(top.left(), bottom.left()));
                        middle.setBottom(bottom.top() - 1);
                        middle.setRight(qMax(top.right(), bottom.right()));
                    }
                }
                else
                {
                    // TopToBottom

                    QRect& left  = first;
                    QRect& right = last;

                    if (left.center().x() > right.center().x())
                    {
                        std::swap(left, right);
                    }

                    int ch = fullSize.height();

                    if      (left.left() != right.left())
                    {
                        // left rectangle

                        if (isRightToLeft())
                        {
                            left.setTop(0);
                        }
                        else
                        {
                            left.setBottom(ch);
                        }

                        // top rectangle

                        if (isRightToLeft())
                        {
                            right.setBottom(ch);
                        }
                        else
                        {
                            right.setTop(0);
                        }

                        // only set middle if the

                        middle.setTop(0);
                        middle.setBottom(ch);
                        middle.setLeft(left.right() + 1);
                        middle.setRight(right.left() - 1);
                    }
                    else if (left.bottom() < right.top())
                    {
                        left.setBottom(right.top() - 1);
                    }
                    else
                    {
                        right.setBottom(left.top() - 1);
                    }
                }

                // get viewport coordinates

                first                          = first.translated( - horizontalOffset(), - verticalOffset());
                middle                         = middle.translated( - horizontalOffset(), - verticalOffset());
                last                           = last.translated( - horizontalOffset(), - verticalOffset());

                // do the selections

                QItemSelection topSelection    = d->selectionForRect(first);
                QItemSelection middleSelection = d->selectionForRect(middle);
                QItemSelection bottomSelection = d->selectionForRect(last);

                // merge

                selection.merge(topSelection, QItemSelectionModel::Select);
                selection.merge(middleSelection, QItemSelectionModel::Select);
                selection.merge(bottomSelection, QItemSelectionModel::Select);
            }
        }
    }

    selectionModel()->select(selection, command);
    d->lastCategorySelection.clear();
}

void DCategorizedView::mouseMoveEvent(QMouseEvent* event)
{
    QListView::mouseMoveEvent(event);

    // was a dragging started?

    if (state() == DraggingState)
    {
        d->mouseButtonPressed      = false;
        d->rightMouseButtonPressed = false;

        if (d->drawItemsWhileDragging)
        {
            viewport()->update(d->lastDraggedItemsRect);
        }
    }

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

    const QModelIndexList item = d->intersectionSet(QRect(event->pos(), event->pos()));

    if (item.count() == 1)
    {
        d->hovered = item[0];
    }
    else
    {
        d->hovered = QModelIndex();
    }

    const QString previousHoveredCategory = d->hoveredCategory;

    d->mousePosition = event->pos();

    d->hoveredCategory.clear();

    // Redraw categories

    for (const QString& category : std::as_const(d->categories))
    {
        if      (d->categoryVisualRect(category).intersects(QRect(event->pos(), event->pos())))
        {
            d->hoveredCategory = category;
            viewport()->update(d->categoryVisualRect(category));
        }
        else if (
                 (category == previousHoveredCategory) &&
                 (!d->categoryVisualRect(previousHoveredCategory).intersects(QRect(event->pos(), event->pos())))
                )
        {
            viewport()->update(d->categoryVisualRect(category));
        }
    }

    QRect rect;

    if (d->mouseButtonPressed && QListView::state() != DraggingState)
    {
        QPoint start, end, initialPressPosition;

        initialPressPosition = d->initialPressPosition;

        initialPressPosition.setY(initialPressPosition.y() - verticalOffset());
        initialPressPosition.setX(initialPressPosition.x() - horizontalOffset());

        if (
            (d->initialPressPosition.x() > d->mousePosition.x()) ||
            (d->initialPressPosition.y() > d->mousePosition.y())
           )
        {
            start = d->mousePosition;
            end   = initialPressPosition;
        }
        else
        {
            start = initialPressPosition;
            end   = d->mousePosition;
        }

        rect = QRect(start, end).adjusted(-16, -16, 16, 16);
        rect = rect.united(QRect(start, end).adjusted(16, 16, -16, -16)).intersected(viewport()->rect());

        viewport()->update(rect);
    }
}

void DCategorizedView::mousePressEvent(QMouseEvent* event)
{
    d->dragLeftViewport = false;

    QListView::mousePressEvent(event);

    if      (event->button() == Qt::LeftButton)
    {
        d->mouseButtonPressed   = true;
        d->initialPressPosition = event->pos();
        d->initialPressPosition.setY(d->initialPressPosition.y() + verticalOffset());
        d->initialPressPosition.setX(d->initialPressPosition.x() + horizontalOffset());
    }
    else if (event->button() == Qt::RightButton)
    {
        d->rightMouseButtonPressed = true;
    }

    if (selectionModel())
    {
        d->lastSelection = selectionModel()->selection();
    }

    viewport()->update(d->categoryVisualRect(d->hoveredCategory));
}

void DCategorizedView::mouseReleaseEvent(QMouseEvent* event)
{
    d->mouseButtonPressed      = false;
    d->rightMouseButtonPressed = false;

    QListView::mouseReleaseEvent(event);

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

    QPoint initPressPos = viewport()->mapFromGlobal(QCursor::pos());
    initPressPos.setY(initPressPos.y() + verticalOffset());
    initPressPos.setX(initPressPos.x() + horizontalOffset());

    if (
        (selectionMode() != SingleSelection) &&
        (selectionMode() != NoSelection)     &&
        (initPressPos == d->initialPressPosition)
       )
    {
        for (const QString& category : std::as_const(d->categories))
        {
            if (
                d->categoryVisualRect(category).contains(event->pos()) &&
                selectionModel()
               )
            {
                QItemSelection selection      = selectionModel()->selection();
                const QVector<int>& indexList = d->categoriesIndexes[category];

                for (int row : std::as_const(indexList))
                {
                    QModelIndex selectIndex = d->proxyModel->index(row, 0);

                    selection << QItemSelectionRange(selectIndex);
                }

                if (d->lastCategorySelection == selection)
                {
                    selectionModel()->select(selection, QItemSelectionModel::Deselect);
                    d->lastCategorySelection.clear();
                }
                else
                {
                    selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
                    d->lastCategorySelection = selection;
                }

                break;
            }
        }
    }

    QRect rect;

    if (state() != DraggingState)
    {
        QPoint start, end, newInitPressPos;

        newInitPressPos = d->initialPressPosition;

        newInitPressPos.setY(newInitPressPos.y() - verticalOffset());
        newInitPressPos.setX(newInitPressPos.x() - horizontalOffset());

        if (
            (d->initialPressPosition.x() > d->mousePosition.x()) ||
            (d->initialPressPosition.y() > d->mousePosition.y())
           )
        {
            start = d->mousePosition;
            end   = newInitPressPos;
        }
        else
        {
            start = newInitPressPos;
            end   = d->mousePosition;
        }

        rect = QRect(start, end).adjusted(-16, -16, 16, 16);
        rect = rect.united(QRect(start, end).adjusted(16, 16, -16, -16)).intersected(viewport()->rect());

        viewport()->update(rect);
    }

    if      (d->hovered.isValid())
    {
        viewport()->update(visualRect(d->hovered));
    }
    else if (!d->hoveredCategory.isEmpty())
    {
        viewport()->update(d->categoryVisualRect(d->hoveredCategory));
    }
}

void DCategorizedView::leaveEvent(QEvent* event)
{
    d->hovered = QModelIndex();
    d->hoveredCategory.clear();
    d->forcedSelectionPosition = 0;
    d->mouseButtonPressed      = false;

    if (state() == DraggingState)
    {
        setState(NoState);
    }

    QListView::leaveEvent(event);
}

void DCategorizedView::startDrag(Qt::DropActions supportedActions)
{
    // FIXME: QAbstractItemView does far better here since it sets the
    //        pixmap of selected icons to the dragging cursor, but it sets a non
    //        ARGB window so it is no transparent. Use QAbstractItemView when
    //        this is fixed on Qt.
    // QAbstractItemView::startDrag(supportedActions);

#if defined(DOLPHIN_DRAGANDDROP)

    Q_UNUSED(supportedActions);

#else

    QListView::startDrag(supportedActions);

#endif

}

void DCategorizedView::dragMoveEvent(QDragMoveEvent* event)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->mousePosition    = event->position().toPoint();

#else

    d->mousePosition    = event->pos();

#endif

    d->dragLeftViewport = false;

#if defined(DOLPHIN_DRAGANDDROP)

    QAbstractItemView::dragMoveEvent(event);

#else

    QListView::dragMoveEvent(event);

#endif

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->hovered = indexAt(event->position().toPoint());

#else

    d->hovered = indexAt(event->pos());

#endif

#if !defined(DOLPHIN_DRAGANDDROP)

    d->drawDraggedItems();

#endif

}

void DCategorizedView::dragLeaveEvent(QDragLeaveEvent* event)
{
    d->dragLeftViewport = true;

#if defined(DOLPHIN_DRAGANDDROP)

    QAbstractItemView::dragLeaveEvent(event);

#else

    QListView::dragLeaveEvent(event);

#endif

}

void DCategorizedView::dropEvent(QDropEvent* event)
{

#if defined(DOLPHIN_DRAGANDDROP)

    QAbstractItemView::dropEvent(event);

#else

    QListView::dropEvent(event);

#endif

}

QModelIndex DCategorizedView::moveCursor(CursorAction cursorAction,
                                         Qt::KeyboardModifiers modifiers)
{
    if (
        (viewMode() != DCategorizedView::IconMode) ||
        !d->proxyModel                             ||
        !d->categoryDrawer                         ||
        d->categories.isEmpty()                    ||
        !d->proxyModel->isCategorizedModel()
       )
    {
        return QListView::moveCursor(cursorAction, modifiers);
    }

    int viewportWidth = viewport()->width() - spacing();
    int itemWidth;

    if (gridSize().isEmpty())
    {
        itemWidth = d->biggestItemSize.width();
    }
    else
    {
        itemWidth = gridSize().width();
    }

    int itemWidthPlusSeparation = spacing() + itemWidth;

    if (!itemWidthPlusSeparation)
    {
        ++itemWidthPlusSeparation;
    }

    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;

    if (!elementsPerRow)
    {
        ++elementsPerRow;
    }

    QModelIndex current = selectionModel() ? selectionModel()->currentIndex()
                                           : QModelIndex();

    if (!current.isValid())
    {
        if (cursorAction == MoveEnd)
        {
            current = model()->index(model()->rowCount() - 1, 0, QModelIndex());
/*
            d->forcedSelectionPosition = d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow;
*/
        }
        else
        {
            current                    = model()->index(0, 0, QModelIndex());
            d->forcedSelectionPosition = 0;
        }

        return current;
    }

    QString lastCategory  = d->categories.first();
    QString theCategory   = d->categories.first();
    QString afterCategory = d->categories.first();
    bool hasToBreak       = false;

    for (const QString& category : std::as_const(d->categories))
    {
        // cppcheck-suppress knownConditionTrueFalse
        if (hasToBreak)
        {
            afterCategory = category;

            break;
        }

        if (category == d->elementsInfo[current.row()].category)
        {
            theCategory = category;
            hasToBreak  = true;
        }

        if (!hasToBreak)
        {
            lastCategory = category;
        }
    }

    switch (cursorAction)
    {
        case QAbstractItemView::MovePageUp:
        {
            // We need to reimplement PageUp/Down as well because
            // default QListView implementation will not work properly with our custom layout

            QModelIndexList visibleIndexes = d->intersectionSet(viewport()->rect());

            if (!visibleIndexes.isEmpty())
            {
                int indexToMove = qMax(current.row() - visibleIndexes.size(), 0);

                return d->proxyModel->index(indexToMove, 0);
            }

            break;
        }

        // fall through

        case QAbstractItemView::MoveUp:
        {
            if (d->elementsInfo[current.row()].relativeOffsetToCategory >= elementsPerRow)
            {
                int indexToMove = current.row();
                indexToMove    -= qMin(((d->elementsInfo[current.row()].relativeOffsetToCategory) +
                                  d->forcedSelectionPosition), elementsPerRow - d->forcedSelectionPosition +
                                  (d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow));

                return (d->proxyModel->index(indexToMove, 0));
            }
            else
            {
                int lastCategoryLastRow = (d->categoriesIndexes[lastCategory].count() - 1) % elementsPerRow;
                int indexToMove         = current.row() - d->elementsInfo[current.row()].relativeOffsetToCategory;

                if (d->forcedSelectionPosition >= lastCategoryLastRow)
                {
                    indexToMove -= 1;
                }
                else
                {
                    indexToMove -= qMin((lastCategoryLastRow - d->forcedSelectionPosition + 1),
                                        d->forcedSelectionPosition + elementsPerRow + 1);
                }

                return d->proxyModel->index(indexToMove, 0);
            }
        }

        case QAbstractItemView::MovePageDown:
        {
            QModelIndexList visibleIndexes = d->intersectionSet(viewport()->rect());

            if (!visibleIndexes.isEmpty())
            {
                int indexToMove = qMin(current.row() + visibleIndexes.size(), d->elementsInfo.size() - 1);

                return d->proxyModel->index(indexToMove, 0);
            }
        }

        // fall through

        case QAbstractItemView::MoveDown:
        {
            if (
                d->elementsInfo[current.row()].relativeOffsetToCategory <
                (d->categoriesIndexes[theCategory].count() - 1 - ((d->categoriesIndexes[theCategory].count() - 1) % elementsPerRow))
               )
            {
                int indexToMove = current.row();
                indexToMove    += qMin(elementsPerRow, d->categoriesIndexes[theCategory].count() - 1 -
                                       d->elementsInfo[current.row()].relativeOffsetToCategory);

                return d->proxyModel->index(indexToMove, 0);
            }
            else
            {
                int afterCategoryLastRow = qMin(elementsPerRow, d->categoriesIndexes[afterCategory].count());
                int indexToMove          = current.row() + (d->categoriesIndexes[theCategory].count() -
                                                            d->elementsInfo[current.row()].relativeOffsetToCategory);

                if (d->forcedSelectionPosition >= afterCategoryLastRow)
                {
                    indexToMove += afterCategoryLastRow - 1;
                }
                else
                {
                    indexToMove += qMin(d->forcedSelectionPosition, elementsPerRow);
                }

                return d->proxyModel->index(indexToMove, 0);
            }
        }

        case QAbstractItemView::MoveLeft:
        {
            if (layoutDirection() == Qt::RightToLeft)
            {
                if (
                    ((current.row() + 1) == d->elementsInfo.size()) ||
                    !(d->elementsInfo[current.row() + 1].relativeOffsetToCategory % elementsPerRow)
                   )
                {
                    return current;
                }

                return d->proxyModel->index(current.row() + 1, 0);
            }

            if (
                (current.row() == 0) ||
                !(d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow)
               )
            {
                return current;
            }

            return d->proxyModel->index(current.row() - 1, 0);
        }

        case QAbstractItemView::MoveRight:
        {
            if (layoutDirection() == Qt::RightToLeft)
            {
                if (
                    (current.row() == 0) ||
                    !(d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow)
                   )
                {
                    return current;
                }

                return d->proxyModel->index(current.row() - 1, 0);
            }

            if (
                ((current.row() + 1) == d->elementsInfo.size()) ||
                !(d->elementsInfo[current.row() + 1].relativeOffsetToCategory % elementsPerRow)
               )
            {
                return current;
            }

            return d->proxyModel->index(current.row() + 1, 0);
        }

        default:
        {
            break;
        }
    }

    return QListView::moveCursor(cursorAction, modifiers);
}

void DCategorizedView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);

    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        d->forcedSelectionPosition = 0;
        d->hovered                 = QModelIndex();
        d->biggestItemSize         = QSize(0, 0);
        d->mouseButtonPressed      = false;
        d->rightMouseButtonPressed = false;
        d->elementsInfo.clear();
        d->elementsPosition.clear();
        d->categoriesIndexes.clear();
        d->categoriesPosition.clear();
        d->categories.clear();
        d->intersectedIndexes.clear();

        return;
    }

    rowsInsertedArtifficial(parent, start, end);
}

int DCategorizedView::Private::categoryUpperBound(SparseModelIndexVector& modelIndexList, int begin, int averageSize)
{
    int end            = modelIndexList.size();
    QString category   = proxyModel->data(modelIndexList[begin],
                                          DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();

    // First case: Small category with <10 entries

    const int smallEnd = qMin(end, begin + 10);

    for (int k = begin ; k < smallEnd ; ++k)
    {
        if (category != proxyModel->data(modelIndexList[k],
                                         DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString())
        {
            return k;
        }
    }

    begin += 10;

    // Second case: only one category, test last value

    QString value = proxyModel->data(modelIndexList[end - 1],
                                     DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();

    if (value == category)
    {
        return end;
    }

    // Third case: use average of last category sizes

    if (averageSize && ((begin + averageSize) < end))
    {
        if      (category != proxyModel->data(modelIndexList[begin + averageSize],
                                              DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString())
        {
            end = begin + averageSize;
        }
        else if ((begin + 2 * averageSize) < end)
        {
            if (category != proxyModel->data(modelIndexList[begin + 2*averageSize],
                                             DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString())
            {
                end = begin + 2 * averageSize;
            }
        }
    }

    // now apply a binary search - the model is sorted by category
    // from qUpperBound, Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    int middle;
    int n = end - begin;
    int half;

    while (n > 0)
    {
        half   = n >> 1;
        middle = begin + half;

        if (category != proxyModel->data(modelIndexList[middle],
                                         DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString())
        {
            n = half;
        }
        else
        {
            begin = middle + 1;
            n    -= half + 1;
        }
    }

    return begin;
}

void DCategorizedView::rowsInsertedArtifficial(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);

    d->forcedSelectionPosition = 0;
    d->hovered                 = QModelIndex();
    d->biggestItemSize         = QSize(0, 0);
    d->mouseButtonPressed      = false;
    d->rightMouseButtonPressed = false;
    d->elementsInfo.clear();
    d->elementsPosition.clear();
    d->categoriesIndexes.clear();
    d->categoriesPosition.clear();
    d->categories.clear();
    d->intersectedIndexes.clear();

    if ((start > end) || (end < 0) || (start < 0) || !d->proxyModel->rowCount())
    {
        return;
    }

    // Add all elements mapped to the source model and explore categories

    const int rowCount   = d->proxyModel->rowCount();
    const int sortColumn = d->proxyModel->sortColumn();
    QString lastCategory = d->proxyModel->data(d->proxyModel->index(0, sortColumn),
                                               DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
    int offset           = -1;

    SparseModelIndexVector modelIndexList(rowCount, d->proxyModel, sortColumn);

    d->elementsInfo      = QVector<Private::ElementInfo>(rowCount);
    int categorySizes    = 0;
    int categoryCounts   = 0;

    if (uniformItemSizes())
    {
        // use last index as sample for size hint

        QModelIndex sample = d->proxyModel->index(rowCount - 1, modelColumn(), rootIndex());
        d->biggestItemSize = sizeHintForIndex(sample);
    }
    else
    {
        QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        initViewItemOption(&option);

#else

        option = viewOptions();

#endif

        for (int k = 0 ; k < rowCount ; ++k)
        {
            QModelIndex indexSize = (sortColumn == 0) ? modelIndexList[k] : d->proxyModel->index(k, 0);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QSize hint            = itemDelegateForIndex(indexSize)->sizeHint(option, indexSize);

#else

            QSize hint            = itemDelegate(indexSize)->sizeHint(option, indexSize);

#endif

            d->biggestItemSize    = QSize(qMax(hint.width(),  d->biggestItemSize.width()),
                                          qMax(hint.height(), d->biggestItemSize.height()));
        }
    }

    for (int k = 0 ; k < rowCount ; )
    {
        lastCategory   = d->proxyModel->data(modelIndexList[k], DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
        int upperBound = d->categoryUpperBound(modelIndexList, k, categorySizes / ++categoryCounts);
        categorySizes += upperBound - k;
        offset         = 0;

        QVector<int> rows(upperBound - k);

        for (int i = k ; i < upperBound ; ++i, ++offset)
        {
            rows[offset]                         = i;
            Private::ElementInfo& elementInfo    = d->elementsInfo[i];
            elementInfo.category                 = lastCategory;
            elementInfo.relativeOffsetToCategory = offset;
        }

        k = upperBound;

        d->categoriesIndexes.insert(lastCategory, rows);
        d->categories << lastCategory;
    }

    d->updateScrollbars();

    // FIXME: We need to safely save the last selection. This is on my TODO
    // list (ereslibre).
    // Note: QItemSelectionModel will save it selection in persistend indexes
    // on layoutChanged(). All works fine for me.
/*
    selectionModel()->clear();
*/
}

void DCategorizedView::rowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (d->proxyModel && d->categoryDrawer && d->proxyModel->isCategorizedModel())
    {
        // Force the view to update all elements

        rowsInsertedArtifficial(QModelIndex(), 0, d->proxyModel->rowCount() - 1);
    }
}

void DCategorizedView::updateGeometries()
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::updateGeometries();
        return;
    }

    // Avoid QListView::updateGeometries(), since it will try to set another
    // range to our scroll bars, what we don't want

    QAbstractItemView::updateGeometries();
}

void DCategorizedView::slotLayoutChanged()
{
    if (d->proxyModel && d->categoryDrawer && d->proxyModel->isCategorizedModel())
    {
        // all cached values are invalidated, recompute immediately

        rowsInsertedArtifficial(QModelIndex(), 0, d->proxyModel->rowCount() - 1);
    }
}

void DCategorizedView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!d->proxyModel || !d->categoryDrawer || !d->proxyModel->isCategorizedModel())
    {
        QListView::currentChanged(current, previous);
        return;
    }

    // We need to update the forcedSelectionPosition property in order to correctly
    // navigate after with keyboard using up & down keys

    int viewportWidth = viewport()->width() - spacing();

//    int itemHeight;
    int itemWidth;

    if (gridSize().isEmpty())
    {
//        itemHeight = d->biggestItemSize.height();
        itemWidth = d->biggestItemSize.width();
    }
    else
    {
//        itemHeight = gridSize().height();
        itemWidth = gridSize().width();
    }

    int itemWidthPlusSeparation = spacing() + itemWidth;

    if (!itemWidthPlusSeparation)
    {
        ++itemWidthPlusSeparation;
    }

    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;

    if (!elementsPerRow)
    {
        ++elementsPerRow;
    }

    if (current.isValid())
    {
        d->forcedSelectionPosition = d->elementsInfo[current.row()].relativeOffsetToCategory % elementsPerRow;
    }

    QListView::currentChanged(current, previous);
}

} // namespace Digikam

#include "moc_dcategorizedview.cpp"
