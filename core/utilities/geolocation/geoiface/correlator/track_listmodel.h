/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-06-09
 * Description : A model to list the tracks
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "trackmanager.h"

namespace Digikam
{

class TrackListModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit TrackListModel(TrackManager* const trackManager, QObject* const parent);
    ~TrackListModel()                                                                                     override;

    // QAbstractItemModel customization:

    int columnCount(const QModelIndex& parent = QModelIndex() )                                     const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role)                               override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                             const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())               const override;
    QModelIndex parent(const QModelIndex& index)                                                    const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                         const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)         override;
    QVariant headerData(int section, Qt::Orientation orientation, int role)                         const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                                   const override;

    TrackManager::Track getTrackForIndex(const QModelIndex& index)                                  const;

private Q_SLOTS:

    void slotTrackManagerUpdated();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam
