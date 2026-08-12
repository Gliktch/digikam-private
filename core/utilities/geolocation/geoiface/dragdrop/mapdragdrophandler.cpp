/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : Drag-and-drop handler for geolocation interface integration.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mapdragdrophandler.h"

// Qt includes

#include <QDropEvent>

// Local includes

#include "gpsgeoifacemodelhelper.h"

namespace Digikam
{

MapDragDropHandler::MapDragDropHandler(QAbstractItemModel* const /*pModel*/,
                                       GPSGeoIfaceModelHelper* const parent)
    : GeoDragDropHandler    (parent),
      gpsGeoIfaceModelHelper(parent)
{
}

Qt::DropAction MapDragDropHandler::accepts(const QDropEvent* /*e*/)
{
    return Qt::CopyAction;
}

bool MapDragDropHandler::dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates)
{
    const MapDragData* const mimeData = qobject_cast<const MapDragData*>(e->mimeData());

    if (!mimeData)
    {
        return false;
    }

    QList<QPersistentModelIndex> droppedIndices;

    for (int i = 0 ; i < mimeData->draggedIndices.count() ; ++i)
    {
        // TODO: correctly handle items with multiple columns

        QModelIndex itemIndex = mimeData->draggedIndices.at(i);

        if (itemIndex.column() == 0)
        {
            droppedIndices << itemIndex;
        }
    }

    gpsGeoIfaceModelHelper->onIndicesMoved(droppedIndices, dropCoordinates, QPersistentModelIndex());

    return true;
}

QMimeData* MapDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& modelIndices)
{
    Q_UNUSED(modelIndices);

    return nullptr;
}

} // namespace Digikam

#include "moc_mapdragdrophandler.cpp"
