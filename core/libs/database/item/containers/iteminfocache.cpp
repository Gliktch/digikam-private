/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ItemInfo common data
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfocache.h"

// Local includes

#include "coredb.h"
#include "coredbalbuminfo.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "iteminfodata.h"
#include "digikam_debug.h"

namespace Digikam
{

ItemInfoCache::ItemInfoCache()
{
    qRegisterMetaType<ItemInfo>("ItemInfo");
    qRegisterMetaType<ItemInfoList>("ItemInfoList");
    qRegisterMetaType<QList<ItemInfo> >("QList<ItemInfo>");

    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChanged(ImageChangeset)),
            Qt::DirectConnection);

    connect(dbwatch, SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChanged(ImageTagChangeset)),
            Qt::DirectConnection);

    connect(dbwatch, SIGNAL(albumChange(AlbumChangeset)),
            this, SLOT(slotAlbumChange(AlbumChangeset)),
            Qt::DirectConnection);
}

static bool lessThanForAlbumShortInfo(const AlbumShortInfo& first, const AlbumShortInfo& second)
{
    return (first.id < second.id);
}

void ItemInfoCache::checkAlbums()
{
    if (m_needUpdateAlbums)
    {
        // list comes sorted from db

        QList<AlbumShortInfo> infos = CoreDbAccess().db()->getAlbumShortInfos();

        ItemInfoWriteLocker lock;
        m_albums                    = infos;
        m_needUpdateAlbums          = false;
    }
}

int ItemInfoCache::getImageGroupedCount(qlonglong id)
{
    if (m_needUpdateGrouped)
    {
        QList<qlonglong> ids = CoreDbAccess().db()->getRelatedImagesToByType(DatabaseRelation::Grouped);

        ItemInfoWriteLocker lock;
        m_grouped            = ids;
        m_needUpdateGrouped  = false;
    }

    ItemInfoReadLocker lock;

    return m_grouped.count(id);
}

QExplicitlySharedDataPointer<ItemInfoData> ItemInfoCache::infoForId(qlonglong id)
{
    {
        ItemInfoReadLocker lock;
        QExplicitlySharedDataPointer<ItemInfoData> ptr(m_infoHash.value(id));

        if (ptr)
        {
            return ptr;
        }
    }

    ItemInfoWriteLocker lock;
    ItemInfoData* const data = new ItemInfoData();
    data->id                 = id;
    m_infoHash[id]           = data;

    return QExplicitlySharedDataPointer<ItemInfoData>(data);
}

void ItemInfoCache::cacheByName(const QExplicitlySharedDataPointer<ItemInfoData>& infoPtr)
{
    // Called with Write lock

    if (!infoPtr || (infoPtr->id == -1) || infoPtr->name.isEmpty())
    {
        return;
    }

    // Called in a context where we can assume that the entry is not yet cached by name (newly created data)

    m_nameHash.remove(m_dataHash.value(infoPtr->id), infoPtr);
    m_dataHash.insert(infoPtr->id, infoPtr->name);
    m_nameHash.insert(infoPtr->name, infoPtr);
}

QExplicitlySharedDataPointer<ItemInfoData> ItemInfoCache::infoForPath(int albumRootId,
                                                                      const QString& relativePath, const QString& name)
{
    ItemInfoReadLocker lock;

    // We check all entries in the multi hash with matching file name

    QMultiHash<QString, QExplicitlySharedDataPointer<ItemInfoData> >::const_iterator it;

    for (it = m_nameHash.constFind(name) ; (it != m_nameHash.constEnd()) && (it.key() == name) ; ++it)
    {
        // first check that album root matches

        if (it.value()->albumRootId != albumRootId)
        {
            continue;
        }

        // check that relativePath matches. We get relativePath from entry's id and compare to given name.

        QList<AlbumShortInfo>::const_iterator albumIt = findAlbum(it.value()->albumId);

        if ((albumIt == m_albums.constEnd()) || (albumIt->relativePath != relativePath))
        {
            continue;
        }

        // we have now a match by name, albumRootId and relativePath

        return it.value();
    }

    return QExplicitlySharedDataPointer<ItemInfoData>();
}

void ItemInfoCache::dropInfo(const QExplicitlySharedDataPointer<ItemInfoData>& infoPtr)
{
    if (!infoPtr)
    {
        return;
    }

    ItemInfoWriteLocker lock;

    // When we have the last ItemInfoData, the reference counter is at 3.
    // Because 2 QExplicitlySharedDataPointers are in cache and 1 is held by m_data.

    if (infoPtr.data()->ref > 3)
    {
        return;
    }

    m_nameHash.remove(m_dataHash.value(infoPtr->id), infoPtr);
    m_nameHash.remove(infoPtr->name, infoPtr);
    m_infoHash.remove(infoPtr->id);
    m_dataHash.remove(infoPtr->id);
}

QList<AlbumShortInfo>::const_iterator ItemInfoCache::findAlbum(int id)
{
    // Called with read lock

    AlbumShortInfo info;
    info.id = id;

    // we use the fact that d->infos is sorted by id

    QList<AlbumShortInfo>::const_iterator it;
    it = std::lower_bound(m_albums.constBegin(),
                          m_albums.constEnd(), info,
                          lessThanForAlbumShortInfo);

    if ((it == m_albums.constEnd()) || (info.id < (*it).id))
    {
        return m_albums.constEnd();
    }

    return it;
}

QString ItemInfoCache::albumRelativePath(int albumId)
{
    checkAlbums();
    ItemInfoReadLocker lock;
    QList<AlbumShortInfo>::const_iterator it = findAlbum(albumId);

    if (it != m_albums.constEnd())
    {
        return it->relativePath;
    }

    return QString();
}

void ItemInfoCache::invalidate()
{
    ItemInfoWriteLocker lock;
    QHash<qlonglong, QExplicitlySharedDataPointer<ItemInfoData> >::iterator it;

    for (it = m_infoHash.begin() ; it != m_infoHash.end() ; ++it)
    {
        (*it)->invalid = true;
        (*it)->id      = -1;
    }

    m_albums.clear();
    m_grouped.clear();
    m_nameHash.clear();
    m_infoHash.clear();
    m_dataHash.clear();
    m_needUpdateAlbums  = true;
    m_needUpdateGrouped = true;
}

void ItemInfoCache::slotImageChanged(const ImageChangeset& changeset)
{
    ItemInfoWriteLocker lock;
    const auto ids = changeset.ids();

    for (const qlonglong& imageId : ids)
    {
        QHash<qlonglong, QExplicitlySharedDataPointer<ItemInfoData> >::iterator it = m_infoHash.find(imageId);

        if (it != m_infoHash.end())
        {
            // invalidate the relevant field. It will be lazy-loaded at first access.

            DatabaseFields::Set changes = changeset.changes();

            if (changes & DatabaseFields::Status)
            {
                (*it)->statusChanged = true;
            }

            if (changes & DatabaseFields::ItemCommentsAll)
            {
                (*it)->defaultCommentCached = false;
                (*it)->defaultTitleCached   = false;
            }

            if (changes & DatabaseFields::Category)
            {
                (*it)->categoryCached = false;
            }

            if (changes & DatabaseFields::Format)
            {
                (*it)->formatCached = false;
            }

            if (changes & DatabaseFields::PickLabel)
            {
                (*it)->pickLabelCached = false;
            }

            if (changes & DatabaseFields::ColorLabel)
            {
                (*it)->colorLabelCached = false;
            }

            if (changes & DatabaseFields::Rating)
            {
                (*it)->ratingCached = false;
            }

            if (changes & DatabaseFields::CreationDate)
            {
                (*it)->creationDateCached = false;
            }

            if (changes & DatabaseFields::ModificationDate)
            {
                (*it)->modificationDateCached = false;
            }

            if (changes & DatabaseFields::Orientation)
            {
                (*it)->orientationCached = false;
            }

            if (changes & DatabaseFields::FileSize)
            {
                (*it)->fileSizeCached = false;
            }

            if (changes & DatabaseFields::UniqueHash)
            {
                (*it)->uniqueHashCached = false;
            }

            if (changes & DatabaseFields::ManualOrder)
            {
                (*it)->manualOrderCached = false;
            }

            if ((changes & DatabaseFields::Width) || (changes & DatabaseFields::Height))
            {
                (*it)->imageSizeCached = false;
            }

            if (
                (changes & DatabaseFields::LatitudeNumber)  ||
                (changes & DatabaseFields::LongitudeNumber) ||
                (changes & DatabaseFields::Altitude)
               )
            {
                (*it)->positionsCached = false;
            }

            if (changes & DatabaseFields::ImageRelations)
            {
                (*it)->groupImageCached = false;
                m_needUpdateGrouped     = true;
            }

            if (changes.hasFieldsFromVideoMetadata())
            {
                const DatabaseFields::VideoMetadata changedVideoMetadata = changes.getVideoMetadata();
                (*it)->videoMetadataCached                              &= ~changedVideoMetadata;
                (*it)->hasVideoMetadata                                  = true;

                (*it)->databaseFieldsHashRaw.removeAllFields(changedVideoMetadata);
            }

            if (changes.hasFieldsFromImageMetadata())
            {
                const DatabaseFields::ImageMetadata changedImageMetadata = changes.getImageMetadata();
                (*it)->imageMetadataCached                              &= ~changedImageMetadata;
                (*it)->hasImageMetadata                                  = true;

                (*it)->databaseFieldsHashRaw.removeAllFields(changedImageMetadata);
            }
        }
        else
        {
            m_needUpdateGrouped = true;
        }
    }
}

void ItemInfoCache::slotImageTagChanged(const ImageTagChangeset& changeset)
{
    if (changeset.propertiesWereChanged())
    {
        ItemInfoWriteLocker lock;
        const auto ids = changeset.ids();

        for (const qlonglong& imageId : ids)
        {
            QHash<qlonglong, QExplicitlySharedDataPointer<ItemInfoData> >::iterator it = m_infoHash.find(imageId);

            if (it != m_infoHash.end())
            {
                (*it)->faceCountCached            = false;
                (*it)->faceSuggestionsCached      = false;
                (*it)->unconfirmedFaceCountCached = false;
            }
        }

        return;
    }

    ItemInfoWriteLocker lock;
    const auto ids = changeset.ids();

    for (const qlonglong& imageId : ids)
    {
        QHash<qlonglong, QExplicitlySharedDataPointer<ItemInfoData> >::iterator it = m_infoHash.find(imageId);

        if (it != m_infoHash.end())
        {
            (*it)->tagIdsCached     = false;
            (*it)->colorLabelCached = false;
            (*it)->pickLabelCached  = false;
        }
    }
}

void ItemInfoCache::slotAlbumChange(const AlbumChangeset& changeset)
{
    switch (changeset.operation())
    {
        case AlbumChangeset::Added:
        case AlbumChangeset::Deleted:
        case AlbumChangeset::Renamed:
        case AlbumChangeset::PropertiesChanged:
        {
            m_needUpdateAlbums = true;
            break;
        }

        case AlbumChangeset::Unknown:
        {
            break;
        }
    }
}

} // namespace Digikam

#include "moc_iteminfocache.cpp"
