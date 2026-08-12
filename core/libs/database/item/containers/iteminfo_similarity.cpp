/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data - similarity
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

double ItemInfo::similarityTo(const qlonglong imageId) const
{
    return imageExtendedProperties().similarityTo(imageId);
}

double ItemInfo::currentSimilarity() const
{
    if (!m_data)
    {
        return 0;
    }

    return m_data->currentSimilarity;
}

qlonglong ItemInfo::currentReferenceImage() const
{
    if (!m_data)
    {
        return -1;
    }

    return m_data->currentReferenceImage;
}

QList<ItemInfo> ItemInfo::fromUniqueHash(const QString& uniqueHash, qlonglong fileSize)
{
    QList<ItemScanInfo> scanInfos = CoreDbAccess().db()->getIdenticalFiles(uniqueHash, fileSize);
    QList<ItemInfo> infos;

    for (const ItemScanInfo& scanInfo : std::as_const(scanInfos))
    {
        infos << ItemInfo(scanInfo.id);
    }

    return infos;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

size_t ItemInfo::hash() const

#else

uint ItemInfo::hash() const

#endif

{
    if (!m_data)
    {
        return ::qHash((int)0);
    }

    return ::qHash(m_data->id);
}

QString ItemInfo::uniqueHash() const
{
    if (!m_data)
    {
        return QString();
    }

    RETURN_IF_CACHED(uniqueHash)

    QVariantList values = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::UniqueHash);

    STORE_IN_CACHE_AND_RETURN(uniqueHash, values.first().toString())
}

} // namespace Digikam
