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

class Q_DECL_HIDDEN AbstractCheckableAlbumModel::Private
{
public:

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    Private()
        : extraFlags                          (),                       // why this difference between Qt5 and 6 ?
          staticVectorContainingCheckStateRole(1, Qt::CheckStateRole)

    {
    }

#else

    Private()
        : extraFlags                          (nullptr),
          staticVectorContainingCheckStateRole(1, Qt::CheckStateRole)

    {
    }

#endif

    Qt::ItemFlags                 extraFlags;
    bool                          rootIsCheckable       = true;
    bool                          addExcludeTristate    = false;
    bool                          recursive             = false;
    QHash<Album*, Qt::CheckState> checkedAlbums;

    QVector<int> staticVectorContainingCheckStateRole;
};

AbstractCheckableAlbumModel::AbstractCheckableAlbumModel(Album::Type albumType,
                                                         Album* const rootAlbum,
                                                         RootAlbumBehavior rootBehavior,
                                                         QObject* const parent)
    : AbstractCountingAlbumModel(albumType, rootAlbum, rootBehavior, parent),
      d                         (new Private)
{
    setup();
}

AbstractCheckableAlbumModel::~AbstractCheckableAlbumModel()
{
    delete d;
}

void AbstractCheckableAlbumModel::setCheckable(bool isCheckable)
{
    if (isCheckable)
    {
        d->extraFlags |= Qt::ItemIsUserCheckable;
    }
    else
    {
        d->extraFlags &= ~Qt::ItemIsUserCheckable;
        resetCheckedAlbums();
    }
}

bool AbstractCheckableAlbumModel::isCheckable() const
{
    return (d->extraFlags & Qt::ItemIsUserCheckable);
}

void AbstractCheckableAlbumModel::setRootCheckable(bool isCheckable)
{
    d->rootIsCheckable = isCheckable;
    Album* const root  = rootAlbum();

    if (!d->rootIsCheckable && root)
    {
        setChecked(root, false);
    }
}

bool AbstractCheckableAlbumModel::rootIsCheckable() const
{
    return (d->rootIsCheckable && isCheckable());
}

void AbstractCheckableAlbumModel::setTristate(bool isTristate)
{
    if (isTristate)
    {
        d->extraFlags |= Qt::ItemIsAutoTristate;
    }
    else
    {
        d->extraFlags &= ~Qt::ItemIsAutoTristate;
    }
}

bool AbstractCheckableAlbumModel::isTristate() const
{
    return (d->extraFlags & Qt::ItemIsAutoTristate);
}

void AbstractCheckableAlbumModel::setAddExcludeTristate(bool b)
{
    d->addExcludeTristate = b;
    setCheckable(true);
    setTristate(b);
}

bool AbstractCheckableAlbumModel::isAddExcludeTristate() const
{
    return (d->addExcludeTristate && isTristate());
}

bool AbstractCheckableAlbumModel::isChecked(Album* album) const
{
    return (d->checkedAlbums.value(album, Qt::Unchecked) == Qt::Checked);
}

Qt::CheckState AbstractCheckableAlbumModel::checkState(Album* album) const
{
    return (d->checkedAlbums.value(album, Qt::Unchecked));
}

void AbstractCheckableAlbumModel::setChecked(Album* album, bool isChecked)
{
    setData(indexForAlbum(album), isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::setCheckState(Album* album, Qt::CheckState state)
{
    setData(indexForAlbum(album), state, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::toggleChecked(Album* album)
{
    if (checkState(album) != Qt::PartiallyChecked)
    {
        setChecked(album, !isChecked(album));
    }
}

QList<Album*> AbstractCheckableAlbumModel::checkedAlbums() const
{
    // return a list with all keys with value Qt::Checked

    return d->checkedAlbums.keys(Qt::Checked);
}

QList<Album*> AbstractCheckableAlbumModel::partiallyCheckedAlbums() const
{
    // return a list with all keys with value Qt::PartiallyChecked

    return d->checkedAlbums.keys(Qt::PartiallyChecked);
}

void AbstractCheckableAlbumModel::setRecursive(bool recursive)
{
    d->recursive = recursive;
}

void AbstractCheckableAlbumModel::resetAllCheckedAlbums()
{
    const QHash<Album*, Qt::CheckState> oldChecked = d->checkedAlbums;
    d->checkedAlbums.clear();

    for (QHash<Album*, Qt::CheckState>::const_iterator it = oldChecked.begin() ;
         it != oldChecked.end() ; ++it)
    {
        if (it.value() != Qt::Unchecked)
        {
            QModelIndex index = indexForAlbum(it.key());

            Q_EMIT dataChanged(index, index, d->staticVectorContainingCheckStateRole);
            Q_EMIT checkStateChanged(it.key(), Qt::Unchecked);
        }
    }
}

void AbstractCheckableAlbumModel::setDataForChildren(const QModelIndex& parent, const QVariant& value, int role)
{
    for (int row = 0 ; row < rowCount(parent) ; ++row)
    {
        QModelIndex childIndex = index(row, 0, parent);
        setData(childIndex, value, role, true);
    }
}

void AbstractCheckableAlbumModel::resetCheckedAlbums(const QModelIndex& parent)
{
    if (parent == rootAlbumIndex())
    {
        resetAllCheckedAlbums();
        return;
    }

    setDataForChildren(parent, Qt::Unchecked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::setDataForParents(const QModelIndex& child, const QVariant& value, int role)
{
    if (!child.isValid())
    {
        return;
    }

    QModelIndex current = parent(child);

    while (current.isValid() && (current != rootAlbumIndex()))
    {
        setData(current, value, role, false);
        current = parent(current);
    }
}

void AbstractCheckableAlbumModel::resetCheckedParentAlbums(const QModelIndex& child)
{
    setDataForParents(child, Qt::Unchecked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::checkAllParentAlbums(const QModelIndex& child)
{
    setDataForParents(child, Qt::Checked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::checkAllAlbums(const QModelIndex& parent)
{
    setDataForChildren(parent, Qt::Checked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::invertCheckedAlbums(const QModelIndex& parent)
{
    Album* const album = albumForIndex(parent);

    if (album)
    {
        toggleChecked(album);
    }

    for (int row = 0 ; row < rowCount(parent) ; ++row)
    {
        invertCheckedAlbums(index(row, 0, parent));
    }
}

void AbstractCheckableAlbumModel::setCheckStateForChildren(Album* album, Qt::CheckState state)
{
    QModelIndex index = indexForAlbum(album);
    setDataForChildren(index, state, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::setCheckStateForParents(Album* album, Qt::CheckState state)
{
    QModelIndex index = indexForAlbum(album);
    setDataForParents(index, state, Qt::CheckStateRole);
}

QVariant AbstractCheckableAlbumModel::albumData(Album* a, int role) const
{
    if (role == Qt::CheckStateRole)
    {
        if (
            (d->extraFlags & Qt::ItemIsUserCheckable) &&
            (!a->isRoot() || d->rootIsCheckable)
           )
        {
            // with Qt::Unchecked as default, albums not in the hash (initially all)
            // are simply regarded as unchecked

            Qt::CheckState state = d->checkedAlbums.value(a, Qt::Unchecked);

            if (d->addExcludeTristate)
            {
                // Use Qt::PartiallyChecked only internally, do not expose it to the TreeView

                return ((state == Qt::Unchecked) ? Qt::Unchecked : Qt::Checked);
            }

            return state;
        }
    }

    return AbstractCountingAlbumModel::albumData(a, role);
}

void AbstractCheckableAlbumModel::prepareAddExcludeDecoration(Album* a, QPixmap& icon) const
{
    if (!d->addExcludeTristate)
    {
        return;
    }

    Qt::CheckState state = checkState(a);

    if (state != Qt::Unchecked)
    {
        int iconSize     = qMax(icon.width(), icon.height());
        int overlay_size = qMin(iconSize, qMax(16, iconSize * 2 / 3));
        QPainter p(&icon);
        p.drawPixmap((icon.width()  - overlay_size) / 2,
                     (icon.height() - overlay_size) / 2,
                     QIcon::fromTheme(state == Qt::PartiallyChecked ? QLatin1String("list-remove")
                                                                    : QLatin1String("list-add")).pixmap(overlay_size, overlay_size));
    }
}

Qt::ItemFlags AbstractCheckableAlbumModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags extraFlags = d->extraFlags;

    if (!d->rootIsCheckable)
    {
        QModelIndex root = rootAlbumIndex();

        if (root.isValid() && (index == root))
        {
            extraFlags &= ~Qt::ItemIsUserCheckable;
        }
    }

    return (AbstractCountingAlbumModel::flags(index) | extraFlags);
}

bool AbstractCheckableAlbumModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return setData(index, value, role, d->recursive);
}

bool AbstractCheckableAlbumModel::setData(const QModelIndex& index, const QVariant& value, int role, bool recursive)
{
    if (role == Qt::CheckStateRole)
    {
        Qt::CheckState state = (Qt::CheckState)value.toInt();
        Album* const album   = albumForIndex(index);

        if (!album)
        {
            return false;
        }
/*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Updating check state for album" << album->title() << "to" << value;
*/
        d->checkedAlbums.insert(album, state);

        Q_EMIT dataChanged(index, index);
        Q_EMIT checkStateChanged(album, state);

        if (recursive)
        {
            setCheckStateForChildren(album, state);
        }

        return true;
    }
    else
    {
        return AbstractCountingAlbumModel::setData(index, value, role);
    }
}

void AbstractCheckableAlbumModel::albumCleared(Album* album)
{
    // preserve check state if album is only being moved

    if (!AlbumManager::instance()->isMovingAlbum(album))
    {
        d->checkedAlbums.remove(album);
    }

    AbstractCountingAlbumModel::albumCleared(album);
}

void AbstractCheckableAlbumModel::allAlbumsCleared()
{
    d->checkedAlbums.clear();
    AbstractCountingAlbumModel::allAlbumsCleared();
}

} // namespace Digikam
