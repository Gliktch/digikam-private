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

#pragma once

// Qt includes

#include <QItemSelectionModel>
#include <QObject>

namespace Digikam
{

class TableViewShared;

class TableViewSelectionModelSyncer : public QObject
{
    Q_OBJECT

public:

    explicit TableViewSelectionModelSyncer(TableViewShared* const sharedObject,
                                           QObject* const parent = nullptr);
    ~TableViewSelectionModelSyncer()                                                   override;

    QModelIndex toSource(const QModelIndex& targetIndex)                         const;
    QModelIndex toTarget(const QModelIndex& sourceIndex)                         const;
    QItemSelection itemSelectionToSource(const QItemSelection& selection)        const;
    QItemSelection itemSelectionToTarget(const QItemSelection& selection)        const;
    int targetModelColumnCount()                                                 const;
    QItemSelection targetIndexToRowItemSelection(const QModelIndex& targetIndex) const;

private Q_SLOTS:

    void slotSourceModelReset();
    void slotSourceCurrentChanged(const QModelIndex& current,
                                  const QModelIndex& previous);
    void slotSourceSelectionChanged(const QItemSelection& selected,
                                    const QItemSelection& deselected);

    void slotTargetCurrentChanged(const QModelIndex& current,
                                  const QModelIndex& previous);
    void slotTargetSelectionChanged(const QItemSelection& selected,
                                    const QItemSelection& deselected);
    void slotTargetColumnsInserted(const QModelIndex& parent,
                                   int start, int end);
    void slotTargetModelRowsInserted(const QModelIndex& parent,
                                     int start, int end);
    void slotTargetModelAboutToBeReset();
    void slotTargetModelReset();

    void slotRestoreFromSavedIds();
    void slotDoInitialSync();

public Q_SLOTS:

    void slotSetActive(const bool isActive);

private:

    class Private;
    const QScopedPointer<Private> d;

    TableViewShared* const        s = nullptr;
};

} // namespace Digikam
