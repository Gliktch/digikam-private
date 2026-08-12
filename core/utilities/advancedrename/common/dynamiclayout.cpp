/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-22
 * Description : a dynamic layout manager
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dynamiclayout.h"

// Qt includes

#include <QStyle>
#include <QWidget>

namespace Digikam
{

class Q_DECL_HIDDEN DynamicLayout::Private
{
public:

    explicit Private(int hSpacing, int vSpacing)
        : hSpace(hSpacing),
          vSpace(vSpacing)
    {
    }

    int                 hSpace       = 0;
    int                 vSpace       = 0;
    int                 spaceX       = 0;
    int                 spaceY       = 0;
    int                 minItemWidth = 0;
    const int           minColumns   = 2;

    QList<QLayoutItem*> itemList;
};

// --------------------------------------------------------

DynamicLayout::DynamicLayout(QWidget* const parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent),
      d      (new Private(hSpacing, vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

DynamicLayout::DynamicLayout(int margin, int hSpacing, int vSpacing)
    : d(new Private(hSpacing, vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

DynamicLayout::~DynamicLayout()
{
    QLayoutItem* item = nullptr;

    while ((item = this->takeAt(0)))
    {
        delete item;
    }

    delete d;
}

void DynamicLayout::addItem(QLayoutItem* layItem)
{
    d->minItemWidth = 0;
    d->itemList.append(layItem);

    for (QLayoutItem* const item : std::as_const(d->itemList))
    {
        QWidget* const wid = item->widget();
        d->spaceX          = qMax<int>(wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton,
                                                                   Qt::Horizontal), d->spaceX);
        d->spaceY          = qMax<int>(wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton,
                                                                   Qt::Vertical), d->spaceY);
        d->minItemWidth    = qMax<int>(wid->sizeHint().width(), d->minItemWidth);
    }

    for (QLayoutItem* const item : std::as_const(d->itemList))
    {
        QWidget* const wid = item->widget();
        wid->setMinimumWidth(d->minItemWidth);
    }
}

int DynamicLayout::horizontalSpacing() const
{
    return d->hSpace;
}

int DynamicLayout::verticalSpacing() const
{
    return d->vSpace;
}

int DynamicLayout::count() const
{
    return d->itemList.size();
}

QLayoutItem* DynamicLayout::itemAt(int index) const
{
    return d->itemList.value(index);
}

QLayoutItem* DynamicLayout::takeAt(int index)
{
    QLayoutItem* item = nullptr;

    if ((index >= 0) && (index < d->itemList.size()))
    {
        item = d->itemList.takeAt(index);
    }

    return item;
}

Qt::Orientations DynamicLayout::expandingDirections() const
{
    return Qt::Orientations();
}

bool DynamicLayout::hasHeightForWidth() const
{
    return true;
}

int DynamicLayout::heightForWidth(int width) const
{
    int height = reLayout(QRect(0, 0, width, 0), true);

    return height;
}

void DynamicLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    reLayout(rect, false);
}

QSize DynamicLayout::sizeHint() const
{
    return minimumSize();
}

QSize DynamicLayout::minimumSize() const
{
    QSize size;

    for (QLayoutItem* const item : std::as_const(d->itemList))
    {
        size = size.expandedTo(item->minimumSize());
    }

    size += QSize(2 * contentsMargins().left(), 2 * contentsMargins().top());
    int w = (size.width() * d->minColumns) + (d->minColumns * d->spaceX);
    size.setWidth(w);

    return size;
}

int DynamicLayout::reLayout(const QRect& rect, bool testOnly) const
{
    int left            = 0;
    int top             = 0;
    int right           = 0;
    int bottom          = 0;
    getContentsMargins(&left, &top, &right, &bottom);

    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x               = effectiveRect.x();
    int y               = effectiveRect.y();
    int lineHeight      = 0;

    // --------------------------------------------------------

    int buttonWidth     = d->minItemWidth + d->spaceX;

    if (buttonWidth == 0)
    {
        buttonWidth = 1;
    }

    int maxButtonsInRow = (effectiveRect.width() - d->spaceX) / buttonWidth;

    if (maxButtonsInRow < d->minColumns)
    {
        maxButtonsInRow = d->minColumns;
    }

    int maxButtonWidth  = d->minItemWidth + ((effectiveRect.width() - (maxButtonsInRow * buttonWidth)) / maxButtonsInRow);

    int currentBtnWidth = (maxButtonsInRow >= d->itemList.count()) ? buttonWidth
                                                                   : maxButtonWidth;

    // --------------------------------------------------------

    for (QLayoutItem* const item : std::as_const(d->itemList))
    {
        int nextX = x + currentBtnWidth + d->spaceX;

        // cppcheck-suppress knownConditionTrueFalse
        if (
            ((nextX - d->spaceX) > effectiveRect.right()) &&
            (lineHeight > 0)
           )
        {
            x          = effectiveRect.x();
            y          = y + lineHeight + d->spaceY;
            nextX      = x + currentBtnWidth  + d->spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
        {
            QSize s = item->sizeHint();
            s.setWidth(currentBtnWidth);
            item->setGeometry(QRect(QPoint(x, y), s));
        }

        x          = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return (y + lineHeight - rect.y() + bottom);
}

} // namespace Digikam

#include "moc_dynamiclayout.cpp"
