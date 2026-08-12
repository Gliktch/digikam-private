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

#include "MarbleGraphicsGridLayout.h"

// Qt includes

#include <QHash>
#include <QRectF>
#include <QSizeF>
#include <QVector>

// Local includes

#include "ScreenGraphicsItem.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN MarbleGraphicsGridLayoutPrivate
{
public:

    MarbleGraphicsGridLayoutPrivate(int rows, int columns)
        : m_rows     (rows),
          m_columns  (columns),
          m_spacing  (0),
          m_alignment(Qt::AlignLeft | Qt::AlignTop)
    {
        m_items = new ScreenGraphicsItem** [rows];

        for (int i = 0 ; i < rows ; ++i)
        {
            m_items[i] = new ScreenGraphicsItem *[columns];
        }

        for (int row = 0 ; row < rows ; row++)
        {
            for (int column = 0 ; column < columns ; column++)
            {
                m_items[row][column] = nullptr;
            }
        }
    }

    ~MarbleGraphicsGridLayoutPrivate()
    {
        for (int i = 0 ; i < m_rows ; ++i)
        {
            delete [] m_items[i];
        }

        delete [] m_items;
    }

public:

    // A two dimensional array of pointers to ScreenGraphicsItems

    ScreenGraphicsItem***                     m_items           = nullptr;
    int                                       m_rows            = 0;
    int                                       m_columns         = 0;
    int                                       m_spacing         = 0;
    Qt::Alignment                             m_alignment       = Qt::AlignLeft | Qt::AlignTop;
    QHash<ScreenGraphicsItem*, Qt::Alignment> m_itemAlignment;

private:

    MarbleGraphicsGridLayoutPrivate(const MarbleGraphicsGridLayoutPrivate&)            = delete;
    MarbleGraphicsGridLayoutPrivate& operator=(const MarbleGraphicsGridLayoutPrivate&) = delete;
};

MarbleGraphicsGridLayout::MarbleGraphicsGridLayout(int rows, int columns)
    : d(new MarbleGraphicsGridLayoutPrivate(rows, columns))
{
}

MarbleGraphicsGridLayout::~MarbleGraphicsGridLayout()
{
    delete d;
}

void MarbleGraphicsGridLayout::addItem(ScreenGraphicsItem* item, int row, int column)
{
    if (
        (row    < d->m_rows) &&
        (column < d->m_columns)
       )
    {
        d->m_items[row][column] = item;
    }
}

void MarbleGraphicsGridLayout::updatePositions(MarbleGraphicsItem* parent)
{
    // Initialize with 0.0

    QVector<double> maxWidth(d->m_columns, 0.0);
    QVector<double> maxHeight(d->m_rows, 0.0);

    // Determining the cell sizes

    for (int row = 0 ; row < d->m_rows ; row++)
    {
        for (int column = 0 ; column < d->m_columns ; column++)
        {
            if (d->m_items[row][column] == nullptr)
            {
                continue;
            }

            QSizeF size   = d->m_items[row][column]->size();
            double width  = size.width();
            double height = size.height();

            if (width > maxWidth[column])
            {
                maxWidth[column] = width;
            }

            if (height > maxHeight[row])
            {
                maxHeight[row] = height;
            }
        }
    }

    QVector<double> startX(d->m_columns);
    QVector<double> endX(d->m_columns);
    QVector<double> startY(d->m_rows);
    QVector<double> endY(d->m_rows);
    QRectF contentRect = parent->contentRect();

    for (int i = 0 ; i < d->m_columns ; i++)
    {
        if      (i == 0)
        {
            startX[0] = contentRect.left();
        }
        else if (maxWidth[i] == 0)
        {
            startX[i] = endX[i - 1];
        }

        else
        {
            startX[i] = endX[i - 1] + d->m_spacing;
        }

        endX[i] = startX[i] + maxWidth[i];
    }

    for (int i = 0 ; i < d->m_rows ; i++)
    {
        if      (i == 0)
        {
            startY[0] = contentRect.left();
        }
        else if (maxHeight[i] == 0)
        {
            startY[i] = endY[i - 1];
        }
        else
        {
            startY[i] = endY[i - 1] + d->m_spacing;
        }

        endY[i] = startY[i] + maxHeight[i];
    }

    // Setting the positions

    for (int row = 0 ; row < d->m_rows ; row++)
    {
        for (int column = 0 ; column < d->m_columns ; column++)
        {
            if (d->m_items[row][column] == nullptr)
            {
                continue;
            }

            double xPos = 0;
            double yPos = 0;

            Qt::Alignment align = alignment(d->m_items[row][column]);

            if      (align & Qt::AlignRight)
            {
                xPos = endX[column] - d->m_items[row][column]->size().width();
            }
            else if (align & Qt::AlignHCenter)
            {
                xPos = startX[column]
                       + (maxWidth[column] - d->m_items[row][column]->size().width()) / 2.0;
            }
            else
            {
                xPos = startX[column];
            }

            if      (align & Qt::AlignBottom)
            {
                yPos = endY[row] - d->m_items[row][column]->size().height();
            }
            else if (align & Qt::AlignVCenter)
            {
                yPos = startY[row]
                       + (maxHeight[row] - d->m_items[row][column]->size().height()) / 2.0;
            }
            else
            {
                yPos = startY[row];
            }

            d->m_items[row][column]->setPosition(QPointF(xPos, yPos));
        }
    }

    parent->setContentSize(QSizeF(endX[d->m_columns - 1] - contentRect.left(),
                                  endY[d->m_rows    - 1] - contentRect.top()));
}

Qt::Alignment MarbleGraphicsGridLayout::alignment() const
{
    return d->m_alignment;
}

Qt::Alignment MarbleGraphicsGridLayout::alignment(ScreenGraphicsItem* item) const
{
    return d->m_itemAlignment.value(item, d->m_alignment);
}

void MarbleGraphicsGridLayout::setAlignment(Qt::Alignment align)
{
    d->m_alignment = align;
}

void MarbleGraphicsGridLayout::setAlignment(ScreenGraphicsItem* item, Qt::Alignment align)
{
    d->m_itemAlignment.insert(item, align);
}

int MarbleGraphicsGridLayout::spacing() const
{
    return d->m_spacing;
}

void MarbleGraphicsGridLayout::setSpacing(int spacing)
{
    d->m_spacing = spacing;
}

} // namespace Marble
