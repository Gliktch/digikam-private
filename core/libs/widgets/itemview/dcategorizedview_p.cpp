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

DCategorizedView::Private::Private(DCategorizedView* const lv)
    : listView(lv)
{
}

const QModelIndexList& DCategorizedView::Private::intersectionSet(const QRect& rect)
{
    QModelIndex index;
    QRect       indexVisualRect;
    int         itemHeight;

    intersectedIndexes.clear();

    if (listView->gridSize().isEmpty())
    {
        itemHeight = biggestItemSize.height();
    }
    else
    {
        itemHeight = listView->gridSize().height();
    }

    // Lets find out where we should start

    int top    = proxyModel->rowCount() - 1;
    int bottom = 0;
    int middle = (top + bottom) / 2;

    while (bottom <= top)
    {
        middle          = (top + bottom) / 2;
        index           = proxyModel->index(middle, 0);
        indexVisualRect = visualRect(index);

        // We need the whole height (not only the visualRect). This will help us to update
        // all needed indexes correctly (ereslibre)

        indexVisualRect.setHeight(indexVisualRect.height() + (itemHeight - indexVisualRect.height()));

        if (
            qMax(indexVisualRect.topLeft().y(),
                 indexVisualRect.bottomRight().y()) < qMin(rect.topLeft().y(),
                                                           rect.bottomRight().y())
           )
        {
            bottom = middle + 1;
        }
        else
        {
            top = middle - 1;
        }
    }

    for (int i = middle ; i < proxyModel->rowCount() ; ++i)
    {
        index           = proxyModel->index(i, 0);
        indexVisualRect = visualRect(index);

        if (rect.intersects(indexVisualRect))
        {
            intersectedIndexes.append(index);
        }

        // If we passed next item, stop searching for hits

        if (
            qMax(rect.bottomRight().y(), rect.topLeft().y()) <
            qMin(indexVisualRect.topLeft().y(), indexVisualRect.bottomRight().y())
           )
        {
            break;
        }
    }

    return intersectedIndexes;
}

QRect DCategorizedView::Private::visualRectInViewport(const QModelIndex& index) const
{
    if (!index.isValid() || (index.row() >= elementsInfo.size()))
    {
        return QRect();
    }

    QRect      retRect;
    QString    curCategory     = elementsInfo[index.row()].category;
    const bool leftToRightFlow = (listView->flow() == QListView::LeftToRight);
    QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    listView->initViewItemOption(&option);

#else

    option = listView->viewOptions();

#endif

    if (leftToRightFlow)
    {
        if (listView->layoutDirection() == Qt::LeftToRight)
        {
            retRect = QRect(listView->spacing(), listView->spacing() * 2 +
                            categoryDrawer->categoryHeight(index, option), 0, 0);
        }
        else
        {
            retRect = QRect(listView->viewport()->width() - listView->spacing(), listView->spacing() * 2 +
                            categoryDrawer->categoryHeight(index, option), 0, 0);
        }
    }
    else
    {
        retRect = QRect(listView->spacing(), listView->spacing() * 2 +
                        categoryDrawer->categoryHeight(index, option), 0, 0);
    }

    int viewportWidth = listView->viewport()->width() - listView->spacing();
    int itemHeight;
    int itemWidth;

    if      (listView->gridSize().isEmpty() && leftToRightFlow)
    {
        itemHeight = biggestItemSize.height();
        itemWidth  = biggestItemSize.width();
    }
    else if (leftToRightFlow)
    {
        itemHeight = listView->gridSize().height();
        itemWidth  = listView->gridSize().width();
    }
    else if (listView->gridSize().isEmpty() && !leftToRightFlow)        // cppcheck-suppress knownConditionTrueFalse
    {
        itemHeight = biggestItemSize.height();
        itemWidth  = listView->viewport()->width() - listView->spacing() * 2;
    }
    else
    {
        itemHeight = listView->gridSize().height();
        itemWidth  = listView->gridSize().width() - listView->spacing() * 2;
    }

    int itemWidthPlusSeparation = listView->spacing() + itemWidth;

    if (!itemWidthPlusSeparation)
    {
        ++itemWidthPlusSeparation;
    }

    int elementsPerRow = viewportWidth / itemWidthPlusSeparation;

    if (!elementsPerRow)
    {
        ++elementsPerRow;
    }

    int column;
    int row;

    if (leftToRightFlow)
    {
        column = elementsInfo[index.row()].relativeOffsetToCategory % elementsPerRow;
        row    = elementsInfo[index.row()].relativeOffsetToCategory / elementsPerRow;

        if (listView->layoutDirection() == Qt::LeftToRight)
        {
            retRect.setLeft(retRect.left() + column * listView->spacing() +
                            column * itemWidth);
        }
        else
        {
            retRect.setLeft(retRect.right() - column * listView->spacing() -
                            column * itemWidth - itemWidth);

            retRect.setRight(retRect.right() - column * listView->spacing() -
                             column * itemWidth);
        }
    }
    else
    {
        elementsPerRow = 1;
        column         = elementsInfo[index.row()].relativeOffsetToCategory % elementsPerRow;
        row            = elementsInfo[index.row()].relativeOffsetToCategory / elementsPerRow;
        (void)column; // Remove clang warnings.
    }

    for (const QString& category : std::as_const(categories))
    {
        if (category == curCategory)
        {
            break;
        }

        float rows  = (float) ((float) categoriesIndexes[category].count() /
                               (float) elementsPerRow);

        int rowsInt = categoriesIndexes[category].count() / elementsPerRow;

        if (rows - trunc(rows))
        {
            ++rowsInt;
        }

        retRect.setTop(retRect.top()                                 +
                       (rowsInt * itemHeight)                        +
                       categoryDrawer->categoryHeight(index, option) +
                       listView->spacing() * 2);

        if (listView->gridSize().isEmpty())
        {
            retRect.setTop(retRect.top() +
                           (rowsInt * listView->spacing()));
        }
    }

    if (listView->gridSize().isEmpty())
    {
        retRect.setTop(retRect.top() + row * listView->spacing() +
                       (row * itemHeight));
    }
    else
    {
        retRect.setTop(retRect.top() + (row * itemHeight));
    }

    retRect.setWidth(itemWidth);

    QModelIndex heightIndex = proxyModel->index(index.row(), 0);

    if (listView->gridSize().isEmpty())
    {
        retRect.setHeight(listView->sizeHintForIndex(heightIndex).height());
    }
    else
    {
        const QSize sizeHint = listView->sizeHintForIndex(heightIndex);

        if (sizeHint.width() < itemWidth && leftToRightFlow)
        {
            retRect.setWidth(sizeHint.width());
            retRect.moveLeft(retRect.left() + (itemWidth - sizeHint.width()) / 2);
        }

        retRect.setHeight(qMin(sizeHint.height(), listView->gridSize().height()));
    }

    return retRect;
}

QRect DCategorizedView::Private::visualCategoryRectInViewport(const QString& category) const
{
    QRect retRect(listView->spacing(),
                  listView->spacing(),
                  listView->viewport()->width() - listView->spacing() * 2,
                  0);

    if (
        !proxyModel                       ||
        !categoryDrawer                   ||
        !proxyModel->isCategorizedModel() ||
        !proxyModel->rowCount()           ||
        !categories.contains(category)
       )
    {
        return QRect();
    }

    QModelIndex index         = proxyModel->index(0, 0, QModelIndex());
    int         viewportWidth = listView->viewport()->width() - listView->spacing();
    int         itemHeight;
    int         itemWidth;

    if (listView->gridSize().isEmpty())
    {
        itemHeight = biggestItemSize.height();
        itemWidth  = biggestItemSize.width();
    }
    else
    {
        itemHeight = listView->gridSize().height();
        itemWidth  = listView->gridSize().width();
    }

    int itemWidthPlusSeparation = listView->spacing() + itemWidth;
    int elementsPerRow          = viewportWidth / itemWidthPlusSeparation;

    if (!elementsPerRow)
    {
        ++elementsPerRow;
    }

    if (listView->flow() == QListView::TopToBottom)
    {
        elementsPerRow = 1;
    }

    QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    listView->initViewItemOption(&option);

#else

    option = listView->viewOptions();

#endif

    for (const QString& itCategory : std::as_const(categories))
    {
        if (itCategory == category)
        {
            break;
        }

        float rows  = (float) ((float) categoriesIndexes[itCategory].count() /
                               (float) elementsPerRow);
        int rowsInt = categoriesIndexes[itCategory].count() / elementsPerRow;

        if (rows - trunc(rows))
        {
            ++rowsInt;
        }

        retRect.setTop(retRect.top()                                 +
                       (rowsInt * itemHeight)                        +
                       categoryDrawer->categoryHeight(index, option) +
                       listView->spacing() * 2);

        if (listView->gridSize().isEmpty())
        {
            retRect.setTop(retRect.top() +
                           (rowsInt * listView->spacing()));
        }
    }

    retRect.setHeight(categoryDrawer->categoryHeight(index, option));

    return retRect;
}

/**
 * We're sure elementsPosition doesn't contain index
 */
const QRect& DCategorizedView::Private::cacheIndex(const QModelIndex& index)
{
    QRect rect                     = visualRectInViewport(index);
    QHash<int, QRect>::iterator it = elementsPosition.insert(index.row(), rect);

    return *it;
}

/**
 * We're sure categoriesPosition doesn't contain category
 */
const QRect& DCategorizedView::Private::cacheCategory(const QString& category)
{
    QRect rect                         = visualCategoryRectInViewport(category);
    QHash<QString, QRect>::iterator it = categoriesPosition.insert(category, rect);

    return *it;
}

const QRect& DCategorizedView::Private::cachedRectIndex(const QModelIndex& index)
{
    QHash<int, QRect>::const_iterator it = elementsPosition.constFind(index.row());

    if (it != elementsPosition.constEnd())   // If we have it cached
    {
        // return it

        return *it;
    }
    else                                     // Otherwise, cache it
    {
        // and return it

        return cacheIndex(index);
    }
}

const QRect& DCategorizedView::Private::cachedRectCategory(const QString& category)
{
    QHash<QString, QRect>::const_iterator it = categoriesPosition.constFind(category);

    if (it != categoriesPosition.constEnd()) // If we have it cached
    {
        // return it

        return *it;
    }
    else                                     // Otherwise, cache it and
    {
        // return it

        return cacheCategory(category);
    }
}

QRect DCategorizedView::Private::visualRect(const QModelIndex& index)
{
    QRect retRect = cachedRectIndex(index);
    int dx        = -listView->horizontalOffset();
    int dy        = -listView->verticalOffset();
    retRect.adjust(dx, dy, dx, dy);

    return retRect;
}

QRect DCategorizedView::Private::categoryVisualRect(const QString& category)
{
    QRect retRect = cachedRectCategory(category);
    int dx        = -listView->horizontalOffset();
    int dy        = -listView->verticalOffset();
    retRect.adjust(dx, dy, dx, dy);

    return retRect;
}

QSize DCategorizedView::Private::contentsSize()
{
    // find the last index in the last category

    QModelIndex lastIndex = categoriesIndexes.isEmpty() ? QModelIndex()
                                                        : proxyModel->index(categoriesIndexes[categories.last()].last(), 0);

    int lastItemBottom    = cachedRectIndex(lastIndex).top() +
                            listView->spacing()              +
                            (listView->gridSize().isEmpty() ? biggestItemSize.height()
                                                            : listView->gridSize().height()) - listView->viewport()->height();

    return QSize(listView->viewport()->width(), lastItemBottom);
}

void DCategorizedView::Private::drawNewCategory(const QModelIndex& index, int sortRole,
                                                const QStyleOption& option, QPainter* painter)
{
    if (!index.isValid())
    {
        return;
    }

    QStyleOption optionCopy = option;
    const QString category  = proxyModel->data(index, DCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
    optionCopy.state       &= ~QStyle::State_Selected;

    if (
        (listView->selectionMode() != SingleSelection) &&
        (listView->selectionMode() != NoSelection)
       )
    {
        if      ((category == hoveredCategory) && !mouseButtonPressed)
        {
            optionCopy.state |= QStyle::State_MouseOver;
        }
        else if ((category == hoveredCategory) && mouseButtonPressed)
        {
            QPoint initialPressPosition = listView->viewport()->mapFromGlobal(QCursor::pos());
            initialPressPosition.setY(initialPressPosition.y() + listView->verticalOffset());
            initialPressPosition.setX(initialPressPosition.x() + listView->horizontalOffset());

            if (initialPressPosition == this->initialPressPosition)
            {
                optionCopy.state |= QStyle::State_Selected;
            }
        }
    }

    categoryDrawer->drawCategory(index, sortRole, optionCopy, painter);
}

void DCategorizedView::Private::updateScrollbars()
{
    listView->horizontalScrollBar()->setRange(0, 0);

    if (listView->verticalScrollMode() == QAbstractItemView::ScrollPerItem)
    {
        listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }

    if (listView->horizontalScrollMode() == QAbstractItemView::ScrollPerItem)
    {
        listView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    }

    listView->verticalScrollBar()->setSingleStep(listView->viewport()->height() / 10);
    listView->verticalScrollBar()->setPageStep(listView->viewport()->height());
    listView->verticalScrollBar()->setRange(0, contentsSize().height());
}

void DCategorizedView::Private::drawDraggedItems(QPainter* painter)
{
    QStyleOptionViewItem option;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    listView->initViewItemOption(&option);

#else

    option        = listView->viewOptions();

#endif

    option.state      &= ~QStyle::State_MouseOver;
    const auto indexes = listView->selectionModel()->selectedIndexes();

    for (const QModelIndex& index : indexes)
    {
        const int dx = mousePosition.x() - initialPressPosition.x() + listView->horizontalOffset();
        const int dy = mousePosition.y() - initialPressPosition.y() + listView->verticalOffset();
        option.rect  = visualRect(index);
        option.rect.adjust(dx, dy, dx, dy);

        if (option.rect.intersects(listView->viewport()->rect()))
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            listView->itemDelegateForIndex(index)->paint(painter, option, index);

#else

            listView->itemDelegate(index)->paint(painter, option, index);

#endif

        }
    }
}

void DCategorizedView::Private::drawDraggedItems()
{
    QRect rectToUpdate;
    QRect currentRect;
    const auto indexes = listView->selectionModel()->selectedIndexes();

    for (const QModelIndex& index : indexes)
    {
        int dx      = mousePosition.x() - initialPressPosition.x() + listView->horizontalOffset();
        int dy      = mousePosition.y() - initialPressPosition.y() + listView->verticalOffset();
        currentRect = visualRect(index);
        currentRect.adjust(dx, dy, dx, dy);

        if (currentRect.intersects(listView->viewport()->rect()))
        {
            rectToUpdate = rectToUpdate.united(currentRect);
        }
    }

    listView->viewport()->update(lastDraggedItemsRect.united(rectToUpdate));

    lastDraggedItemsRect = rectToUpdate;
}

} // namespace Digikam
