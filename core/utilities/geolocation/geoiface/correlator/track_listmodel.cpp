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

#include "track_listmodel.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes


namespace Digikam
{

enum TrackColumns
{
    ColumnVisible  = 0,
    ColumnNPoints  = 1,
    ColumnFilename = 2,
    ColumnCount    = 3
};

class Q_DECL_HIDDEN TrackListModel::Private
{
public:

    Private() = default;

    TrackManager* trackManager = nullptr;
};

TrackListModel::TrackListModel(TrackManager* const trackManager, QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->trackManager = trackManager;

    connect(d->trackManager, SIGNAL(signalAllTrackFilesReady()),
            this, SLOT(slotTrackManagerUpdated()));
}

TrackListModel::~TrackListModel()
{
}

int TrackListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return ColumnCount;
}

bool TrackListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

QVariant TrackListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    const int rowNumber    = index.row();
    const int columnNumber = index.column();

    if (
        (columnNumber < 0) || (columnNumber >= ColumnCount) ||
        (rowNumber < 0)    || (rowNumber >= d->trackManager->trackCount())
       )
    {
        return QVariant();
    }

    TrackManager::Track track = d->trackManager->getTrackById(index.internalId());

    if (track.id == 0)
    {
        // track not found, invalid id

        return QVariant();
    }

    switch (role)
    {
        case Qt::DisplayRole:
        {
            switch (columnNumber)
            {
                case ColumnFilename:
                {
                    return track.url.fileName();
                }

                case ColumnNPoints:
                {
                    return QString::number(track.points.count());
                }

                case ColumnVisible:
                {
                    return QString();
                }
            }

            break;
        }

        case Qt::BackgroundRole:
        case Qt::DecorationRole:
        {
            switch (columnNumber)
            {
                case ColumnVisible:
                {
                    return track.color;
                }
            }

            break;
        }
    }

    return QVariant();
}

QModelIndex TrackListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Q_ASSERT(parent.model() == this);

        return QModelIndex();
    }

    if (
        (column < 0) || (column >= ColumnCount) ||
        (row < 0)    || (row >= d->trackManager->trackCount())
       )
    {
        return QModelIndex();
    }

    const TrackManager::Track track = d->trackManager->getTrack(row);

    /// @todo We have to use quint32 for track ids

    return createIndex(row, column, quint32(track.id));
}

QModelIndex TrackListModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model()==this);
    }

    // we have only top level items

    return QModelIndex();
}

int TrackListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        Q_ASSERT(parent.model() == this);

        return 0;
    }

    return d->trackManager->trackCount();
}

bool TrackListModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

QVariant TrackListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((section >= ColumnCount) || (orientation != Qt::Horizontal) )
    {
        return false;
    }

    if (role != Qt::DisplayRole)
    {
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    /// @todo Make color and #points sections more narrow
    /// @todo Display visible state and make it toggable

    switch (section)
    {
        case ColumnVisible:
        {
            return i18nc("@title: track list header", "Color");
        }

        case ColumnFilename:
        {
            return i18nc("@title: track list header", "Filename");
        }

        case ColumnNPoints:
        {
            return i18nc("@title: track list header", "#points");
        }
    }

    return false;
}

Qt::ItemFlags TrackListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    else
    {
        Q_ASSERT(index.model() == this);
    }

    return QAbstractItemModel::flags(index);
}

void Digikam::TrackListModel::slotTrackManagerUpdated()
{
    /// @todo The TrackManager should send more detailed reports

    beginResetModel();
    endResetModel();
}

TrackManager::Track TrackListModel::getTrackForIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return d->trackManager->getTrackById(index.internalId());
    }

    return TrackManager::Track();
}

} // namespace Digikam

#include "moc_track_listmodel.cpp"
