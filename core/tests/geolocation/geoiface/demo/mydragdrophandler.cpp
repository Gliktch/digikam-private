/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for geolocation interface used in the demo
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mydragdrophandler.h"

// Qt includes

#include <QDropEvent>

// Local includes

#include "digikam_debug.h"
#include "mytreewidget.h"

MyDragDropHandler::MyDragDropHandler(QAbstractItemModel* const pModel, QObject* const parent)
    : GeoDragDropHandler(parent),
      model             (pModel)
{
}

MyDragDropHandler::~MyDragDropHandler()
{
}

Qt::DropAction MyDragDropHandler::accepts(const QDropEvent* /*e*/)
{
    return Qt::CopyAction;
}

bool MyDragDropHandler::dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates)
{
    const MyDragData* const mimeData = qobject_cast<const MyDragData*>(e->mimeData());

    if (!mimeData)
        return false;

    qCDebug(DIGIKAM_TESTS_LOG) << mimeData->draggedIndices.count();

    for (int i = 0 ; i < mimeData->draggedIndices.count() ; ++i)
    {
        const QPersistentModelIndex itemIndex = mimeData->draggedIndices.at(i);

        if (!itemIndex.isValid())
        {
            continue;
        }

        model->setData(itemIndex, QVariant::fromValue(dropCoordinates), RoleCoordinates);
    }

    // TODO: tell the main window about this so it can start an altitude lookup

    return true;
}

QMimeData* MyDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& /*modelIndices*/)
{
    return nullptr;
}

#include "moc_mydragdrophandler.cpp"
