/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-18
 * Description : Sync QItemSelectionModel of ItemFilterModel and TableViewModel
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2026      by Anamay Narkar <anamay dot narkar dot 102 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tableview_selection_model_syncer.h"

// Qt includes

#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "itemfiltermodel.h"
#include "tableview_model.h"
#include "tableview_shared.h"

namespace Digikam
{

class Q_DECL_HIDDEN TableViewSelectionModelSyncer::Private
{
public:

    Private() = default;

public:

    bool             syncing              = false;

    /**
     * @note Snapshot taken in slotTargetModelAboutToBeReset. storing image IDs
     *       rather than model indexes, which become stale
     *       after a model reset.
     */

    QList<qlonglong> savedImageIds;
    qlonglong        savedCurrentImageId  = -1;
    bool             hasSavedSelection    = false;
};

TableViewSelectionModelSyncer::TableViewSelectionModelSyncer(TableViewShared* const sharedObject,
                                                             QObject* const parent)
    : QObject(parent),
      d      (new Private),
      s      (sharedObject)
{
    connect(s->imageFilterSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSourceCurrentChanged(QModelIndex,QModelIndex)));

    connect(s->imageFilterSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSourceSelectionChanged(QItemSelection,QItemSelection)));

    connect(s->tableViewSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotTargetCurrentChanged(QModelIndex,QModelIndex)));

    connect(s->tableViewSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotTargetSelectionChanged(QItemSelection,QItemSelection)));

    connect(s->tableViewModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(slotTargetColumnsInserted(QModelIndex,int,int)));

    connect(s->tableViewModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotTargetModelAboutToBeReset()));

    connect(s->tableViewModel, SIGNAL(modelReset()),
            this, SLOT(slotTargetModelReset()));

    connect(s->tableViewModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotTargetModelRowsInserted(QModelIndex,int,int)));

    /**
     * @todo This is necessary to re-sync the selection when tags are added to images.
     *       Check whether both are necessary or whether we need more.
     */

    connect(s->imageFilterModel, SIGNAL(layoutChanged()),
            this, SLOT(slotSourceModelReset()));

    connect(s->imageFilterModel, SIGNAL(modelReset()),
            this, SLOT(slotSourceModelReset()));

    slotDoInitialSync();
}

TableViewSelectionModelSyncer::~TableViewSelectionModelSyncer()
{
}

QModelIndex TableViewSelectionModelSyncer::toSource(const QModelIndex& tableViewIndex) const
{
    return s->tableViewModel->toItemFilterModelIndex(tableViewIndex);
}

QModelIndex TableViewSelectionModelSyncer::toTarget(const QModelIndex& sourceIndex) const
{
    return s->tableViewModel->fromItemFilterModelIndex(sourceIndex);
}

int TableViewSelectionModelSyncer::targetModelColumnCount() const
{
    return s->tableViewModel->columnCount(QModelIndex());
}

QItemSelection TableViewSelectionModelSyncer::targetIndexToRowItemSelection(const QModelIndex& targetIndex) const
{
    const int row                 = targetIndex.row();
    const QModelIndex topLeft     = s->tableViewModel->index(row, 0, targetIndex.parent());
    const QModelIndex bottomRight = s->tableViewModel->index(row, targetModelColumnCount()-1, targetIndex.parent());
    const QItemSelection mySelection(topLeft, bottomRight);

    return mySelection;
}

void TableViewSelectionModelSyncer::slotDoInitialSync()
{
    if (!s->isActive)
    {
        return;
    }

    d->syncing                           = true;

    s->tableViewSelectionModel->clearSelection();

    const QItemSelection sourceSelection = s->imageFilterSelectionModel->selection();
    const QItemSelection targetSelection = itemSelectionToTarget(sourceSelection);
    s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);

    const QModelIndex targetIndexCurrent = toTarget(s->imageFilterSelectionModel->currentIndex());
    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::NoUpdate);

    d->syncing                           = false;
}

void TableViewSelectionModelSyncer::slotSourceCurrentChanged(const QModelIndex& current,
                                                             const QModelIndex& previous)
{
    if (!s->isActive)
    {
        return;
    }

    Q_UNUSED(previous)

    if (d->syncing)
    {
        return;
    }

    d->syncing                           = true;

    // we have to select the whole row of the target index

    const QModelIndex targetIndexCurrent = toTarget(current);

    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::Select);

    d->syncing                           = false;
}

QItemSelection TableViewSelectionModelSyncer::itemSelectionToSource(const QItemSelection& selection) const
{
    QItemSelection sourceSelection;

    for (const QItemSelectionRange& range : std::as_const(selection))
    {
        const int firstRow = range.top();
        const int lastRow  = range.bottom();

        for (int row = firstRow ; row <= lastRow ; ++row)
        {
            const QModelIndex tableViewIndex = s->tableViewModel->index(row, 0, range.parent());
            const QModelIndex sourceIndex    = s->tableViewModel->toItemFilterModelIndex(tableViewIndex);

            if (sourceIndex.isValid())
            {
                sourceSelection.select(sourceIndex, sourceIndex);
            }
        }
    }

    return sourceSelection;
}

QItemSelection TableViewSelectionModelSyncer::itemSelectionToTarget(const QItemSelection& selection) const
{
    const int      targetColumnCount = targetModelColumnCount();
    QItemSelection targetSelection;

    for (const QItemSelectionRange& range : std::as_const(selection))
    {
        const int firstRow = range.top();
        const int lastRow  = range.bottom();

        for (int row = firstRow ; row <= lastRow ; ++row)
        {
            const QModelIndex sourceIndex               = s->imageFilterModel->index(row, 0, range.parent());
            const QModelIndex tableViewIndexTopLeft     = s->tableViewModel->fromItemFilterModelIndex(sourceIndex);
            const QModelIndex tableViewIndexBottomRight = s->tableViewModel->index(tableViewIndexTopLeft.row(),
                                                                                   targetColumnCount-1,
                                                                                   tableViewIndexTopLeft.parent());

            targetSelection.select(tableViewIndexTopLeft, tableViewIndexBottomRight);
        }
    }

    return targetSelection;
}

void TableViewSelectionModelSyncer::slotSourceSelectionChanged(const QItemSelection& selected,
                                                               const QItemSelection& deselected)
{
    if (!s->isActive)
    {
        return;
    }

    if (d->syncing)
    {
        return;
    }

    d->syncing                             = true;

    const QItemSelection targetSelection   = itemSelectionToTarget(selected);
    s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);

    const QItemSelection targetDeselection = itemSelectionToTarget(deselected);
    s->tableViewSelectionModel->select(targetDeselection, QItemSelectionModel::Deselect);

    d->syncing                             = false;
}

void TableViewSelectionModelSyncer::slotTargetCurrentChanged(const QModelIndex& current,
                                                             const QModelIndex& previous)
{
    if (!s->isActive)
    {
        return;
    }

    Q_UNUSED(previous)

    if (d->syncing)
    {
        return;
    }

    d->syncing                           = true;

    const QModelIndex sourceIndexCurrent = toSource(current);
    s->imageFilterSelectionModel->setCurrentIndex(sourceIndexCurrent, QItemSelectionModel::Select);

    d->syncing                           = false;
}

void TableViewSelectionModelSyncer::slotTargetSelectionChanged(const QItemSelection& selected,
                                                               const QItemSelection& deselected)
{
    if (!s->isActive)
    {
        return;
    }

    if (d->syncing)
    {
        return;
    }

    d->syncing                             = true;

    const QItemSelection sourceSelection   = itemSelectionToSource(selected);
    s->imageFilterSelectionModel->select(sourceSelection, QItemSelectionModel::Select);

    const QItemSelection sourceDeselection = itemSelectionToSource(deselected);
    s->imageFilterSelectionModel->select(sourceDeselection, QItemSelectionModel::Deselect);

    d->syncing                             = false;
}

void TableViewSelectionModelSyncer::slotSourceModelReset()
{
    // QAbstractItemModel will also react to the modelReset signal
    // make sure we transfer the update after that.

    QTimer::singleShot(0, this, SLOT(slotDoInitialSync()));
}

void TableViewSelectionModelSyncer::slotTargetColumnsInserted(const QModelIndex& parent,
                                                              int start, int end)
{
    if (!s->isActive)
    {
        return;
    }

    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    if (d->syncing)
    {
        return;
    }

    /**
     * @note New columns were inserted. We have to make sure that all selected rows include the new columns.
     * We just re-perform the initial synchronization.
     *
     * @todo There may be more efficient ways.
     */

    slotDoInitialSync();
}

void Digikam::TableViewSelectionModelSyncer::slotSetActive(const bool isActive)
{
    if (isActive)
    {
        slotSourceModelReset();
    }
}

void Digikam::TableViewSelectionModelSyncer::slotTargetModelAboutToBeReset()
{
    d->savedImageIds.clear();
    d->savedCurrentImageId = -1;

    const QModelIndexList selected = s->tableViewSelectionModel->selectedIndexes();

    for (const QModelIndex& idx : std::as_const(selected))
    {
        if (idx.column() == 0)
        {
            const qlonglong id = s->tableViewModel->imageId(idx);

            if (id > 0)
            {
                d->savedImageIds.append(id);
            }
        }
    }

    const QModelIndex currentTarget = s->tableViewSelectionModel->currentIndex();

    if (currentTarget.isValid())
    {
        d->savedCurrentImageId = s->tableViewModel->imageId(currentTarget);
    }

    d->hasSavedSelection = true;
    d->syncing           = true;
}

void Digikam::TableViewSelectionModelSyncer::slotTargetModelReset()
{
    d->syncing = false;

    if (!d->hasSavedSelection)
    {
        slotDoInitialSync();
        return;
    }

    // imageFilterModel may itself be mid-reset when tableViewModel fires modelReset,
    // Deferring the restore after a tick to let all pending updates complete/get queued for completion.

    QTimer::singleShot(0, this, SLOT(slotRestoreFromSavedIds()));
}

void Digikam::TableViewSelectionModelSyncer::slotRestoreFromSavedIds()
{
    if (!d->hasSavedSelection)
    {
        return;
    }

    d->hasSavedSelection = false;

    if (!s->isActive || d->savedImageIds.isEmpty())
    {
        return;
    }

    d->syncing = true;

    s->tableViewSelectionModel->clearSelection();
    s->imageFilterSelectionModel->clearSelection();

    for (const qlonglong imageId : std::as_const(d->savedImageIds))
    {
        const QModelIndex targetIndex = s->tableViewModel->indexFromImageId(imageId, 0);

        if (!targetIndex.isValid())
        {
            continue;
        }

        const QItemSelection rowSel   = targetIndexToRowItemSelection(targetIndex);
        s->tableViewSelectionModel->select(rowSel, QItemSelectionModel::Select);

        const QModelIndex sourceIndex = toSource(targetIndex);

        if (sourceIndex.isValid())
        {
            s->imageFilterSelectionModel->select(sourceIndex, QItemSelectionModel::Select);
        }
    }

    if (d->savedCurrentImageId > 0)
    {
        const QModelIndex targetCurrent = s->tableViewModel->indexFromImageId(d->savedCurrentImageId, 0);

        if (targetCurrent.isValid())
        {
            s->tableViewSelectionModel->setCurrentIndex(targetCurrent, QItemSelectionModel::NoUpdate);

            const QModelIndex sourceCurrent = toSource(targetCurrent);
            s->imageFilterSelectionModel->setCurrentIndex(sourceCurrent, QItemSelectionModel::NoUpdate);
        }
    }

    d->syncing = false;
}

void Digikam::TableViewSelectionModelSyncer::slotTargetModelRowsInserted(const QModelIndex& parent,
                                                                         int start, int end)
{
    if (!s->isActive)
    {
        return;
    }

    // look up the state of the source indexes and transfer them here

    for (int i = start ; i <= end ; ++i)
    {
        const QModelIndex iTarget = s->tableViewModel->index(i, 0, parent);

        if (!iTarget.isValid())
        {
            continue;
        }

        const QModelIndex iSource = toSource(iTarget);

        if (!iSource.isValid())
        {
            continue;
        }

        if (s->imageFilterSelectionModel->isSelected(iSource))
        {
            const QItemSelection targetSelection = targetIndexToRowItemSelection(iTarget);
            s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);
        }
    }

    // also transfer the current index if necessary

    const QModelIndex sourceCurrentIndex = s->imageFilterSelectionModel->currentIndex();
    const QModelIndex targetIndexCurrent = toTarget(sourceCurrentIndex);
    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::NoUpdate);
}

} // namespace Digikam

#include "moc_tableview_selection_model_syncer.cpp"
