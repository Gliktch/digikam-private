/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data - Groups
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfo_p.h"

namespace Digikam
{

bool ItemInfo::hasGroupedImages() const
{
    return numberOfGroupedImages();
}

int ItemInfo::numberOfGroupedImages() const
{
    if (!m_data)
    {
        return 0;
    }

    return ItemInfoStatic::cache()->getImageGroupedCount(m_data->id);
}

qlonglong ItemInfo::groupImageId() const
{
    if (!m_data)
    {
        return -1;
    }

    RETURN_IF_CACHED(groupImage)

    QList<qlonglong> ids = CoreDbAccess().db()->getImagesRelatedFrom(m_data->id, DatabaseRelation::Grouped);

    // list size should be 0 or 1

    int groupImg         = ids.isEmpty() ? -1 : ids.first();

    ItemInfoWriteLocker lock;
    m_data.data()->groupImage       = groupImg;
    m_data.data()->groupImageCached = true;

    return m_data->groupImage;
}

bool ItemInfo::isGrouped() const
{
    return (groupImageId() != -1);
}

ItemInfo ItemInfo::groupImage() const
{
    qlonglong gid = groupImageId();

    if (gid == -1)
    {
        return ItemInfo();
    }

    return ItemInfo(gid);
}

QList<ItemInfo> ItemInfo::groupedImages() const
{
    if (!m_data || !hasGroupedImages())
    {
        return QList<ItemInfo>();
    }

    return ItemInfoList(CoreDbAccess().db()->getImagesRelatingTo(m_data->id, DatabaseRelation::Grouped));
}

void ItemInfo::addToGroup(const ItemInfo& givenLeader)
{
    if (!m_data || givenLeader.isNull() || (givenLeader.id() == m_data->id))
    {
        return;
    }

    // Take care: Once we start this, we cannot rely on change notifications and cache invalidation!

    CoreDbOperationGroup group;

    // Handle grouping on an already grouped image, and prevent circular grouping

    ItemInfo leader;
    QList<qlonglong> alreadySeen;
    alreadySeen << m_data->id;

    for (leader = givenLeader ; leader.isGrouped() ; )
    {
        ItemInfo nextLeader = leader.groupImage();

        // is the new leader currently grouped on this image, or do we have a circular grouping?

        if (alreadySeen.contains(nextLeader.id()))
        {
            // break loop (special case: remove b->a where we want to add a->b)

            leader.removeFromGroup();
            break;
        }
        else
        {
            alreadySeen << leader.id();
            leader = nextLeader;
        }
    }

    // Already grouped correctly?

    if (groupImageId() == leader.id())
    {
        return;
    }

    // All images grouped on this image need a new group leader

    QList<qlonglong> idsToBeGrouped  = CoreDbAccess().db()->getImagesRelatingTo(m_data->id, DatabaseRelation::Grouped);

    // and finally, this image needs to be grouped

    idsToBeGrouped << m_data->id;

    for (const qlonglong& ids : std::as_const(idsToBeGrouped))
    {
        // remove current grouping

        CoreDbAccess().db()->removeAllImageRelationsFrom(ids, DatabaseRelation::Grouped);

        // add the new grouping

        CoreDbAccess().db()->addImageRelation(ids, leader.id(), DatabaseRelation::Grouped);
    }
}

void ItemInfo::removeFromGroup()
{
    if (!m_data)
    {
        return;
    }

    if (!isGrouped())
    {
        return;
    }

    CoreDbAccess().db()->removeAllImageRelationsFrom(m_data->id, DatabaseRelation::Grouped);
}

void ItemInfo::clearGroup()
{
    if (!m_data)
    {
        return;
    }

    if (!hasGroupedImages())
    {
        return;
    }

    CoreDbAccess().db()->removeAllImageRelationsTo(m_data->id, DatabaseRelation::Grouped);
}

} // namespace Digikam
