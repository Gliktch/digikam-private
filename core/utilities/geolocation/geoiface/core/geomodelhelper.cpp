/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Helper class to access models
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "geomodelhelper.h"

namespace Digikam
{

/**
 * @class GeoModelHelper
 * @brief Helper class to access data in models.
 *
 * @c GeoModelHelper is used to access data held in models, which is not suitable for transfer using the
 * the Qt-style API, like coordinates or custom sized thumbnails.
 *
 * The basic functions which have to be implemented are:
 * @li model(): Returns a pointer to the model
 * @li selectionModel(): Returns a pointer to the selection model. It may return a null-pointer
 *     if no selection model is used.
 * @li itemCoordinates(): Returns the coordinates for a given item index, if it has any.
 * @li modelFlags(): Returns flags for the model.
 *
 * For ungrouped models, the following functions should also be implemented:
 * @li itemIcon(): Returns an icon for an index, and an offset to the 'center' of the item.
 * @li itemFlags(): Returns flags for individual items.
 * @li snapItemsTo(): Grouped items have been moved and should snap to an index.
 *
 * For grouped models which are accessed by @c MarkerModel, the following functions should be implemented:
 * @li bestRepresentativeIndexFromList(): Find the item that should represent a group of items.
 * @li pixmapFromRepresentativeIndex(): Find a thumbnail for an item.
 */

GeoModelHelper::GeoModelHelper(QObject* const parent)
    : QObject(parent)
{
}

void GeoModelHelper::snapItemsTo(const QModelIndex& targetIndex,
                                 const QList<QPersistentModelIndex>& snappedIndices)
{
    QList<QModelIndex> result;

    for (int i = 0; i < snappedIndices.count(); ++i)
    {
        result << snappedIndices.at(i);
    }

    snapItemsTo(targetIndex, result);
}

QPersistentModelIndex GeoModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                   const int /*sortKey*/)
{
    // this is only a stub to provide some default implementation

    if (list.isEmpty())
    {
        return QPersistentModelIndex();
    }

    return list.first();
}

/**
 * @brief Returns the icon for an ungrouped marker.
 *
 * The icon can either be returned as a URL to an image, or as a QPixmap. If the caller
 * can handle URLs (for example, to display them in HTML), he can provide the URL parameter.
 * However, the GeoModelHelper may still choose to return a QPixmap instead, if no URL is
 * available.
 *
 * @param index Modelindex of the marker.
 * @param offset Offset of the zero point in the icon, given from the top-left.
 * @param size the size of the icon, only populated if a URL is returned.
 * @param pixmap Holder for the pixmap of the icon.
 * @param url URL of the icon if available.
 */
bool GeoModelHelper::itemIcon(const QModelIndex& index,
                              QPoint* const offset,
                              QSize* const size,
                              QPixmap* const pixmap,
                              QUrl* const url) const
{
    Q_UNUSED(index)
    Q_UNUSED(offset)
    Q_UNUSED(size)
    Q_UNUSED(pixmap)
    Q_UNUSED(url)

    return false;
}

GeoModelHelper::PropertyFlags GeoModelHelper::modelFlags() const
{
    return PropertyFlags();
}

GeoModelHelper::PropertyFlags GeoModelHelper::itemFlags(const QModelIndex& /*index*/) const
{
    return PropertyFlags();
}

void GeoModelHelper::snapItemsTo(const QModelIndex& /*targetIndex*/,
                                 const QList<QModelIndex>& /*snappedIndices*/)
{
}

QPixmap GeoModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& /*index*/,
                                                      const QSize& /*size*/)
{
    return QPixmap();
}

void GeoModelHelper::onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices)
{
    Q_UNUSED(clickedIndices);
}

void GeoModelHelper::onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices,
                                    const GeoCoordinates& targetCoordinates,
                                    const QPersistentModelIndex& targetSnapIndex)
{
    Q_UNUSED(movedIndices);
    Q_UNUSED(targetCoordinates);
    Q_UNUSED(targetSnapIndex);
}

} // namespace Digikam

#include "moc_geomodelhelper.cpp"
