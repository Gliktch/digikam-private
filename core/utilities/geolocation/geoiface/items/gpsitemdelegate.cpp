/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A model for the view to display a list of items.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsitemdelegate.h"

// Qt includes


#include <QPainter>
#include <QIcon>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN GPSItemDelegate::Private
{
public:

    Private() = default;

public:

    GPSItemList* imageList      = nullptr;
    int          thumbnailSize  = 60;
};

GPSItemDelegate::GPSItemDelegate(GPSItemList* const imageList, QObject* const parent)
    : QItemDelegate(parent),
      d            (new Private)
{
    d->imageList = imageList;
}

GPSItemDelegate::~GPSItemDelegate()
{
    delete d;
}

void GPSItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const
{
    if (sortMappedindex.column() != GPSItemContainer::ColumnThumbnail)
    {
        QItemDelegate::paint(painter, option, sortMappedindex);

        return;
    }

    const QModelIndex& sourceModelIndex = d->imageList->getSortProxyModel()->mapToSource(sortMappedindex);

    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    // TODO: clipping, selected state, disabled state, etc.

    QPixmap itemPixmap = d->imageList->getModel()->getPixmapForIndex(sourceModelIndex, d->thumbnailSize);

    if (itemPixmap.isNull())
    {
        // TODO: paint some default logo
        // TODO: cache this logo

        itemPixmap = QIcon::fromTheme(QLatin1String("view-preview"))
                                      .pixmap(d->thumbnailSize, QIcon::Disabled);
    }

    const QSize availableSize = option.rect.size();
    const QSize pixmapSize    = itemPixmap.size().boundedTo(availableSize);
    QPoint startPoint((availableSize.width()  - pixmapSize.width())  / 2,
                      (availableSize.height() - pixmapSize.height()) / 2);
    startPoint               += option.rect.topLeft();
    painter->drawPixmap(QRect(startPoint, pixmapSize), itemPixmap, QRect(QPoint(0, 0), pixmapSize));
}

QSize GPSItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const
{
    if (sortMappedindex.column() == GPSItemContainer::ColumnThumbnail)
    {
        return QSize(d->thumbnailSize, d->thumbnailSize);
    }

    const QSize realSizeHint = QItemDelegate::sizeHint(option, sortMappedindex);

    return QSize(realSizeHint.width(), d->thumbnailSize);
}

void GPSItemDelegate::setThumbnailSize(const int size)
{
    d->thumbnailSize               = size;
    const GPSItemModel* const imageModel = d->imageList->getModel();

    if (!imageModel)
    {
        return;
    }

    if (imageModel->rowCount() > 0)
    {
        // TODO: is it enough to Q_EMIT this signal for only 1 item?
        // seems to work in Qt4.5 with QTreeView::setUniformRowHeights(true)

        Q_EMIT sizeHintChanged(imageModel->index(0, 0));
    }
}

int GPSItemDelegate::getThumbnailSize() const
{
    return d->thumbnailSize;
}

} // namespace Digikam

#include "moc_gpsitemdelegate.cpp"
