/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data
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

ThumbnailIdentifier ItemInfo::thumbnailIdentifier() const
{
    if (!m_data)
    {
        return ThumbnailIdentifier();
    }

    ThumbnailIdentifier tid;
    tid.id       = m_data->id;
    tid.filePath = filePath();

    return tid;
}

ThumbnailInfo ItemInfo::thumbnailInfo() const
{
    if (!m_data)
    {
        return ThumbnailInfo();
    }

    ThumbnailInfo thumbinfo;

    thumbinfo.id               = m_data->id;
    thumbinfo.filePath         = filePath();
    thumbinfo.fileName         = name();
    thumbinfo.isAccessible     = CollectionManager::instance()->locationForAlbumRootId(m_data->albumRootId).isAvailable();
    thumbinfo.modificationDate = modDateTime();
    thumbinfo.orientationHint  = orientation();
    thumbinfo.uniqueHash       = uniqueHash();
    thumbinfo.fileSize         = fileSize();

    if      (category() == DatabaseItem::Image)
    {
        thumbinfo.mimeType = QLatin1String("image");
    }
    else if (category() == DatabaseItem::Video)
    {
        thumbinfo.mimeType = QLatin1String("video");
    }

    return thumbinfo;
}

ThumbnailIdentifier ItemInfo::thumbnailIdentifier(qlonglong id)
{
    ItemInfo info(id);

    return info.thumbnailIdentifier();
}

} // namespace Digikam
