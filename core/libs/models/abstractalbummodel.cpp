/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-23
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractalbummodel_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN AbstractAlbumModel::Private
{
public:

    Private() = default;

public:

    Album*                                rootAlbum         = nullptr;
    Album*                                addingAlbum       = nullptr;
    Album*                                removingAlbum     = nullptr;
    Album::Type                           type              = Album::PHYSICAL;
    AlbumModelDragDropHandler*            dragDropHandler   = nullptr;
    AbstractAlbumModel::RootAlbumBehavior rootBehavior      = AbstractAlbumModel::IncludeRootAlbum;

    bool                                  itemDrag          = true;
    bool                                  itemDrop          = true;
    bool                                  isFaceTagModel    = false;

    QModelIndex                           dropIndex;
};

AbstractAlbumModel::AbstractAlbumModel(Album::Type albumType,
                                       Album* const rootAlbum,
                                       RootAlbumBehavior rootBehavior,
                                       QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->type         = albumType;
    d->rootAlbum    = rootAlbum;
    d->rootBehavior = rootBehavior;

    // --- NOTE: use dynamic binding as all slots above are virtual methods which can be re-implemented in derived classes.

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumAboutToBeAdded,
            this, &AbstractAlbumModel::slotAlbumAboutToBeAdded);

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumAdded,
            this, &AbstractAlbumModel::slotAlbumAdded);

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumAboutToBeDeleted,
            this, &AbstractAlbumModel::slotAlbumAboutToBeDeleted);

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumHasBeenDeleted,
            this, &AbstractAlbumModel::slotAlbumHasBeenDeleted);

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumsCleared,
            this, &AbstractAlbumModel::slotAlbumsCleared);

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumIconChanged,
            this, &AbstractAlbumModel::slotAlbumIconChanged);

    connect(AlbumManager::instance(), &AlbumManager::signalAlbumRenamed,
            this, &AbstractAlbumModel::slotAlbumRenamed);
}

AbstractAlbumModel::~AbstractAlbumModel()
{
    delete d;
}

QVariant AbstractAlbumModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (d->dropIndex.isValid() && (index == d->dropIndex))
    {
        if      (role == Qt::BackgroundRole)
        {
            return QPalette().highlight();
        }
        else if (role == Qt::ForegroundRole)
        {
            return QPalette().highlightedText();
        }
    }

    Album* const a = static_cast<Album*>(index.internalPointer());

    return albumData(a, role);
}

QVariant AbstractAlbumModel::albumData(Album* a, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
        {
            return a->title();
        }

        case Qt::ToolTipRole:
        {
            return a->title();
        }

        case Qt::DecorationRole:
        {
            // reimplemented in subclasses

            return decorationRoleData(a);
        }

        case Qt::FontRole:
        {
            return fontRoleData(a);
        }

        case AlbumTitleRole:
        {
            return a->title();
        }

        case AlbumTypeRole:
        {
            return a->type();
        }

        case AlbumPointerRole:
        {
            return QVariant::fromValue(a);
        }

        case AlbumIdRole:
        {
            return a->id();
        }

        case AlbumGlobalIdRole:
        {
            return a->globalID();
        }

        case AlbumSortRole:
        {
            // reimplemented in subclass

            return sortRoleData(a);
        }

        default:
        {
            return QVariant();
        }
    }
}

QVariant AbstractAlbumModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if ((section == 0) && (role == Qt::DisplayRole))
    {
        return columnHeader();
    }

    return QVariant();
}

int AbstractAlbumModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        const Album* const a = static_cast<Album*>(parent.internalPointer());

        return a->childCount();
    }
    else
    {
        if (!d->rootAlbum)
        {
            return 0;
        }

        if (d->rootBehavior == IncludeRootAlbum)
        {
            return 1;
        }
        else
        {
            return d->rootAlbum->childCount();
        }
    }
}

int AbstractAlbumModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

Qt::ItemFlags AbstractAlbumModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Album* const a = static_cast<Album*>(index.internalPointer());

    return itemFlags(a);
}

bool AbstractAlbumModel::hasChildren(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        const Album* const a = static_cast<Album*>(parent.internalPointer());

        return a->firstChild();
    }
    else
    {
        if (!d->rootAlbum)
        {
            return false;
        }

        if (d->rootBehavior == IncludeRootAlbum)
        {
            return 1;
        }
        else
        {
            return d->rootAlbum->firstChild();
        }
    }
}

QModelIndex AbstractAlbumModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column != 0) || (row < 0))
    {
        return QModelIndex();
    }

    if (parent.isValid())
    {
        const Album* const parentAlbum = static_cast<Album*>(parent.internalPointer());
        Album* const a                 = parentAlbum->childAtRow(row);

        if (a)
        {
            return createIndex(row, column, a);
        }
    }
    else
    {
        if (!d->rootAlbum)
        {
            return QModelIndex();
        }

        if (d->rootBehavior == IncludeRootAlbum)
        {
            if (row == 0)
            {
                return createIndex(0, 0, d->rootAlbum);
            }
        }
        else
        {
            Album* const a = d->rootAlbum->childAtRow(row);

            if (a)
            {
                return createIndex(row, column, a);
            }
        }
    }

    return QModelIndex();
}

QModelIndex AbstractAlbumModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        const Album* const a = static_cast<Album*>(index.internalPointer());

        return indexForAlbum(a->parent());
    }

    return QModelIndex();
}

Qt::DropActions AbstractAlbumModel::supportedDropActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}

QStringList AbstractAlbumModel::mimeTypes() const
{
    if (d->dragDropHandler)
    {
        return d->dragDropHandler->mimeTypes();
    }

    return QStringList();
}

bool AbstractAlbumModel::dropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&)
{
    // we require custom solutions

    return false;
}

QMimeData* AbstractAlbumModel::mimeData(const QModelIndexList& indexes) const
{
    if (!d->dragDropHandler)
    {
        return nullptr;
    }

    QList<Album*> albums;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        Album* const a = albumForIndex(ind);             // cppcheck-suppress constVariablePointer

        if (a)
        {
            albums << a;
        }
    }

    return d->dragDropHandler->createMimeData(albums);
}

void AbstractAlbumModel::setEnableDrag(bool enable)
{
    d->itemDrag = enable;
}

void AbstractAlbumModel::setEnableDrop(bool enable)
{
    d->itemDrop = enable;
}

void AbstractAlbumModel::setDropIndex(const QModelIndex& index)
{
    const QModelIndex oldIndex = d->dropIndex;
    d->dropIndex               = index;

    if (!index.isValid() && oldIndex.isValid())
    {
        Q_EMIT dataChanged(oldIndex, oldIndex);
    }
}

void AbstractAlbumModel::setDragDropHandler(AlbumModelDragDropHandler* handler)
{
    d->dragDropHandler = handler;
}

AlbumModelDragDropHandler* AbstractAlbumModel::dragDropHandler() const
{
    return d->dragDropHandler;
}

void AbstractAlbumModel::setFaceTagModel(bool enable)
{
    d->isFaceTagModel = enable;
}

QModelIndex AbstractAlbumModel::indexForAlbum(Album* a) const
{
    if (!a)
    {
        return QModelIndex();
    }

    if (!filterAlbum(a))
    {
        return QModelIndex();
    }

    // a is root album? Decide on root behavior

    if (a == d->rootAlbum)
    {
        if (d->rootBehavior == IncludeRootAlbum)
        {
            // create top-level indexes

            return createIndex(0, 0, a);
        }
        else
        {
            // with this behavior, root album has no valid index

            return QModelIndex();
        }
    }

    // Normal album. Get its row.

    return createIndex(a->rowFromAlbum(), 0, a);
}

Album* AbstractAlbumModel::albumForIndex(const QModelIndex& index) const
{
    return (static_cast<Album*>(index.internalPointer()));
}

Album* AbstractAlbumModel::retrieveAlbum(const QModelIndex& index)
{
    return (index.data(AbstractAlbumModel::AlbumPointerRole).value<Album*>());
}

Album* AbstractAlbumModel::rootAlbum() const
{
    return d->rootAlbum;
}

QModelIndex AbstractAlbumModel::rootAlbumIndex() const
{
    return indexForAlbum(d->rootAlbum);
}

AbstractAlbumModel::RootAlbumBehavior AbstractAlbumModel::rootAlbumBehavior() const
{
    return d->rootBehavior;
}

Album::Type AbstractAlbumModel::albumType() const
{
    return d->type;
}

bool AbstractAlbumModel::isFaceTagModel() const
{
    return d->isFaceTagModel;
}

QVariant AbstractAlbumModel::decorationRoleData(Album*) const
{
    return QVariant();
}

QVariant AbstractAlbumModel::fontRoleData(Album*) const
{
    return QVariant();
}

QVariant AbstractAlbumModel::sortRoleData(Album* a) const
{
    return a->title();
}

QString AbstractAlbumModel::columnHeader() const
{
    return i18n("Album");
}

Qt::ItemFlags AbstractAlbumModel::itemFlags(Album*) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if (d->itemDrag)
    {
        f |= Qt::ItemIsDragEnabled;
    }

    if (d->itemDrop)
    {
        f |= Qt::ItemIsDropEnabled;
    }

    return f;
}

bool AbstractAlbumModel::filterAlbum(Album* album) const
{
    return (album && (album->type() == d->type));
}

void AbstractAlbumModel::slotAlbumAboutToBeAdded(Album* album, Album* parent, Album* prev)
{
    if (!filterAlbum(album))
    {
        return;
    }

    if (album->isRoot() && (d->rootBehavior == IgnoreRootAlbum))
    {
        d->rootAlbum = album;

        return;
    }

    // start inserting operation

    int row                 = prev ? prev->rowFromAlbum() + 1 : 0;
    QModelIndex parentIndex = indexForAlbum(parent);
    beginInsertRows(parentIndex, row, row);

    // The root album will become available in time
    // when the model is instantiated before albums are initialized.
    // Set d->rootAlbum only after

    if (album->isRoot() && !d->rootAlbum)
    {
        d->rootAlbum = album;
    }

    // store album for slotAlbumAdded

    d->addingAlbum = album;
}

void AbstractAlbumModel::slotAlbumAdded(Album* album)
{
    if (d->addingAlbum == album)
    {
        bool isRoot    = (d->addingAlbum == d->rootAlbum);
        d->addingAlbum = nullptr;
        endInsertRows();

        if (isRoot)
        {
            Q_EMIT rootAlbumAvailable();
        }
    }
}

void AbstractAlbumModel::slotAlbumAboutToBeDeleted(Album* album)
{
    if (!filterAlbum(album))
    {
        return;
    }

    if (album->isRoot() && (d->rootBehavior == IgnoreRootAlbum))
    {
        albumCleared(album);
        d->rootAlbum = nullptr;

        return;
    }

    // begin removing operation

    int row          = album->rowFromAlbum();
    QModelIndex prnt = indexForAlbum(album->parent());
    beginRemoveRows(prnt, row, row);
    albumCleared(album);

    // store album for slotAlbumHasBeenDeleted

    d->removingAlbum = album;
}

void AbstractAlbumModel::slotAlbumHasBeenDeleted(Album* album)
{
    if (d->removingAlbum == album)
    {
        d->removingAlbum = nullptr;
        endRemoveRows();
    }
}

void AbstractAlbumModel::slotAlbumsCleared()
{
    d->rootAlbum = nullptr;
    beginResetModel();
    allAlbumsCleared();
    endResetModel();
}

void AbstractAlbumModel::slotAlbumIconChanged(Album* album)
{
    if (!filterAlbum(album))
    {
        return;
    }

    QModelIndex ind = indexForAlbum(album);

    Q_EMIT dataChanged(ind, ind);
}

void AbstractAlbumModel::slotAlbumRenamed(Album* album)
{
    if (!filterAlbum(album))
    {
        return;
    }

    QModelIndex ind = indexForAlbum(album);

    Q_EMIT dataChanged(ind, ind);
}

} // namespace Digikam

#include "moc_abstractalbummodel.cpp"
