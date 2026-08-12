/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries, using AlbumManager
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemalbumfiltermodel.h"

// Qt includes

#include <QTimer>

// Local includes

#include "itemfiltermodel_p.h"
#include "album.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "itemalbummodel.h"
#include "facetagsiface.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemAlbumFilterModelPrivate : public ItemFilterModel::ItemFilterModelPrivate
{
    Q_OBJECT

public:

    ItemAlbumFilterModelPrivate() = default;

public:

    QHash<int, QString> tagNamesHash;
    QHash<int, QString> albumNamesHash;
    QTimer*             delayedAlbumNamesTimer  = nullptr;
    QTimer*             delayedTagNamesTimer    = nullptr;

private:

    // Disable
    ItemAlbumFilterModelPrivate(QObject*);
};

ItemAlbumFilterModel::ItemAlbumFilterModel(QObject* const parent)
    : ItemFilterModel(*new ItemAlbumFilterModelPrivate, parent)
{
    Q_D(ItemAlbumFilterModel);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    d->delayedAlbumNamesTimer = new QTimer(this);
    d->delayedAlbumNamesTimer->setInterval(250);
    d->delayedAlbumNamesTimer->setSingleShot(true);

    d->delayedTagNamesTimer = new QTimer(this);
    d->delayedTagNamesTimer->setInterval(250);
    d->delayedTagNamesTimer->setSingleShot(true);

    connect(d->delayedAlbumNamesTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedAlbumNamesTimer()));

    connect(d->delayedTagNamesTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedTagNamesTimer()));

    const auto palbums = AlbumManager::instance()->allPAlbums();

    for (Album* const a : palbums)
    {
        albumChange(a);
    }

    const auto talbums = AlbumManager::instance()->allTAlbums();

    for (Album* const a : talbums)
    {
        albumChange(a);
    }
}

ItemAlbumFilterModel::~ItemAlbumFilterModel()
{
}

void ItemAlbumFilterModel::setSourceItemModel(ItemAlbumModel* model)
{
    ItemFilterModel::setSourceItemModel(model);
}

ItemAlbumModel* ItemAlbumFilterModel::sourceModel() const
{
    Q_D(const ItemAlbumFilterModel);

    return static_cast<ItemAlbumModel*>(d->imageModel);
}

void ItemAlbumFilterModel::setItemFilterSettings(const ItemFilterSettings& s)
{
    Q_D(ItemAlbumFilterModel);               // cppcheck-suppress constVariablePointer

    ItemFilterSettings settings(s);
    settings.setAlbumNames(d->albumNamesHash);
    settings.setTagNames(d->tagNamesHash);
    ItemFilterModel::setItemFilterSettings(settings);
}

int ItemAlbumFilterModel::compareInfosCategories(const ItemInfo& left, const ItemInfo& right) const
{
    FaceTagsIface leftFace, rightFace;

    return compareInfosCategories(left, right, leftFace, rightFace);
}

int ItemAlbumFilterModel::compareInfosCategories(const ItemInfo& left, const ItemInfo& right,
                                                 const FaceTagsIface& leftFace, const FaceTagsIface& rightFace) const
{
    Q_D(const ItemAlbumFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case ItemSortSettings::CategoryByAlbum:
        {
            int leftAlbumId                = left.albumId();
            int rightAlbumId               = right.albumId();
            const PAlbum* const leftAlbum  = AlbumManager::instance()->findPAlbum(leftAlbumId);
            const PAlbum* const rightAlbum = AlbumManager::instance()->findPAlbum(rightAlbumId);

            if (!leftAlbum || !rightAlbum)
            {
                return -1;
            }

            if (leftAlbum == rightAlbum)
            {
                return 0;
            }

            if (
                (d->sorter.sortRole == ItemSortSettings::SortByCreationDate) ||
                (d->sorter.sortRole == ItemSortSettings::SortByModificationDate)
               )
            {
                // Here we want to sort the categories by album date if images are sorted by date.
                // We must still make sure that categorization is unique!

                QDate leftDate  = leftAlbum->date();
                QDate rightDate = rightAlbum->date();

                if (leftDate != rightDate)
                {

                    return ItemSortSettings::compareByOrder(leftDate > rightDate ? 1 : -1,
                                                            d->sorter.currentCategorizationSortOrder);
                }
            }

            // Make album paths unique across different collections.
            // The album root path can be empty if the collection is offline.

            QString leftPath  = leftAlbum->albumPath()  + QLatin1Char('/') + QString::number(leftAlbum->rowFromAlbum())
                                                        + QLatin1Char('/') + QString::number(leftAlbum->id());
            QString rightPath = rightAlbum->albumPath() + QLatin1Char('/') + QString::number(rightAlbum->rowFromAlbum())
                                                        + QLatin1Char('/') + QString::number(rightAlbum->id());

            return ItemSortSettings::naturalCompare(leftPath, rightPath,
                                                    d->sorter.currentCategorizationSortOrder,
                                                    d->sorter.categorizationCaseSensitivity,
                                                    d->sorter.strTypeNatural);
        }

        default:
        {
            return ItemFilterModel::compareInfosCategories(left, right, leftFace, rightFace);
        }
    }
}

void ItemAlbumFilterModel::albumChange(Album* album)
{
    Q_D(const ItemAlbumFilterModel);

    if      (album->type() == Album::PHYSICAL)
    {
        d->delayedAlbumNamesTimer->start();
    }
    else if (album->type() == Album::TAG)
    {
        d->delayedTagNamesTimer->start();
    }
}

void ItemAlbumFilterModel::slotAlbumRenamed(Album* album)
{
    albumChange(album);
}

void ItemAlbumFilterModel::slotAlbumAdded(Album* album)
{
    albumChange(album);
}

void ItemAlbumFilterModel::slotAlbumAboutToBeDeleted(Album* album)
{
    albumChange(album);
}

void ItemAlbumFilterModel::slotAlbumsCleared()
{
    Q_D(ItemAlbumFilterModel);

    d->albumNamesHash.clear();
    d->tagNamesHash.clear();
}

void ItemAlbumFilterModel::slotDelayedAlbumNamesTimer()
{
    Q_D(ItemAlbumFilterModel);

    d->albumNamesHash = AlbumManager::instance()->albumTitles();

    if (d->filter.isFilteringByText())
    {
        setItemFilterSettings(d->filter);
    }
}

void ItemAlbumFilterModel::slotDelayedTagNamesTimer()
{
    Q_D(ItemAlbumFilterModel);

    d->tagNamesHash = AlbumManager::instance()->tagNames();

    if (d->filter.isFilteringByText() || d->filter.isFilteringByTags())
    {
        setItemFilterSettings(d->filter);
    }
}

} // namespace Digikam

#include "itemalbumfiltermodel.moc"

#include "moc_itemalbumfiltermodel.cpp"
