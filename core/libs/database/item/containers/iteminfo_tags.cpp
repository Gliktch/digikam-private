/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data - Tags
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

void ItemInfo::setTag(int tagID)
{
    if (!m_data || (tagID <= 0))
    {
        return;
    }

    bool newTag = !tagIds().contains(tagID);
    CoreDbAccess().db()->addItemTag(m_data->id, tagID, newTag);
}

void ItemInfo::removeTag(int tagID)
{
    if (!m_data)
    {
        return;
    }

    CoreDbAccess access;
    access.db()->removeItemTag(m_data->id, tagID);
    access.db()->removeImageTagProperties(m_data->id, tagID);
}

void ItemInfo::removeAllTags()
{
    if (!m_data)
    {
        return;
    }

    CoreDbAccess().db()->removeItemAllTags(m_data->id, tagIds());
}

void ItemInfo::addTagPaths(const QStringList& tagPaths)
{
    if (!m_data)
    {
        return;
    }

    QList<int> tids = TagsCache::instance()->tagsForPaths(tagPaths);
    CoreDbAccess().db()->addTagsToItems(QList<qlonglong>() << m_data->id, tids);
}

QList<int> ItemInfo::tagIds() const
{
    if (!m_data)
    {
        return QList<int>();
    }

    RETURN_IF_CACHED(tagIds)

    QList<int> ids = CoreDbAccess().db()->getItemTagIDs(m_data->id);

    ItemInfoWriteLocker lock;
    m_data.data()->tagIds       = ids;
    m_data.data()->tagIdsCached = true;

    return ids;
}

ItemTagPair ItemInfo::imageTagPair(int tagId) const
{
    if (!m_data)
    {
        return ItemTagPair();
    }

    return ItemTagPair(*this, tagId);
}

QList<ItemTagPair> ItemInfo::availableItemTagPairs() const
{
    if (!m_data)
    {
        return QList<ItemTagPair>();
    }

    return ItemTagPair::availablePairs(*this);
}

} // namespace Digikam
