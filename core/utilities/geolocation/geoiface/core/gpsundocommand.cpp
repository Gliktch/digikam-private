/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-25
 * Description : A class to hold undo/redo commands.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsundocommand.h"

// Local includes

#include "gpsitemmodel.h"

namespace Digikam
{

GPSUndoCommand::GPSUndoCommand(QUndoCommand* const parent)
    : QUndoCommand(parent)
{
}

void GPSUndoCommand::changeItemData(const bool redoIt)
{
    if (undoList.isEmpty())
    {
        return;
    }

    // get a pointer to the GPSItemModel:
    // TODO: why is the model returned as const?

    GPSItemModel* const imageModel = const_cast<GPSItemModel*>(dynamic_cast<const GPSItemModel*>(undoList.first().modelIndex.model()));

    if (!imageModel)
    {
        return;
    }

    for (int i = 0 ; i < undoList.count() ; ++i)
    {
        const UndoInfo& info         = undoList.at(i);
        GPSItemContainer* const item = imageModel->itemFromIndex(info.modelIndex);

        // TODO: correctly handle the dirty flags
        // TODO: find a way to regenerate tag tree

        GPSDataContainer newData     = redoIt ? info.dataAfter : info.dataBefore;
        item->restoreGPSData(newData);
        QList<QList<TagData> > newRGTagList = redoIt ? info.newTagList : info.oldTagList;
        item->restoreRGTagList(newRGTagList);
    }
}

void GPSUndoCommand::redo()
{
    changeItemData(true);
}

void GPSUndoCommand::undo()
{
    changeItemData(false);
}

void GPSUndoCommand::addUndoInfo(const UndoInfo& info)
{
    undoList << info;
}

void GPSUndoCommand::UndoInfo::readOldDataFromItem(const GPSItemContainer* const imageItem)
{
    this->dataBefore = imageItem->gpsData();
    this->oldTagList = imageItem->getTagList();
}

void GPSUndoCommand::UndoInfo::readNewDataFromItem(const GPSItemContainer* const imageItem)
{
    this->dataAfter  = imageItem->gpsData();
    this->newTagList = imageItem->getTagList();
}

} // namespace Digikam
