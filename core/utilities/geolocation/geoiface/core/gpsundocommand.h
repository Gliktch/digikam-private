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

#pragma once

// Qt includes

#include <QUndoCommand>

// Local includes

#include "gpsitemcontainer.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GPSUndoCommand : public QUndoCommand
{
public:

    class DIGIKAM_EXPORT UndoInfo
    {
    public:

        explicit UndoInfo(const QPersistentModelIndex& pModelIndex)
            : modelIndex(pModelIndex)
        {
        }

        void readOldDataFromItem(const GPSItemContainer* const imageItem);
        void readNewDataFromItem(const GPSItemContainer* const imageItem);

    public:

        QPersistentModelIndex   modelIndex;
        GPSDataContainer        dataBefore;
        GPSDataContainer        dataAfter;

        QList<QList<TagData> >  oldTagList;
        QList<QList<TagData> >  newTagList;

        typedef QList<UndoInfo> List;
    };

    explicit GPSUndoCommand(QUndoCommand* const parent = nullptr);

    void addUndoInfo(const UndoInfo& info);
    void changeItemData(const bool redoIt);

    inline int affectedItemCount() const
    {
        return undoList.count();
    }

    void redo() override;
    void undo() override;

private:

    UndoInfo::List undoList;
};

} // namespace Digikam
