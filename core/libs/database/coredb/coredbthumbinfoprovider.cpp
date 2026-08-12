/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-06-08
 * Description : Core database <-> thumbnail database interface
 *
 * SPDX-FileCopyrightText: 2009      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbthumbinfoprovider.h"

// Qt includes

#include <QUrl>

// Local includes

#include "coredb.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "coredbaccess.h"
#include "iteminfo.h"
#include "thumbnailcreator.h"

namespace Digikam
{

ThumbnailInfo ThumbsDbInfoProvider::thumbnailInfo(const ThumbnailIdentifier& identifier)
{
    // If code here proves to be a bottleneck we can add custom queries to albumdb to retrieve info all-in-one

    ItemInfo imageinfo;

    if (identifier.id)
    {
        imageinfo = ItemInfo(identifier.id);
    }
    else
    {
        imageinfo = ItemInfo::fromLocalFile(identifier.filePath);
    }

    if (imageinfo.isNull())
    {
        return ThumbnailCreator::fileThumbnailInfo(identifier.filePath);
    }

    return imageinfo.thumbnailInfo();
}

// ---------------------------------------------------------------------------------

int DatabaseLoadSaveFileInfoProvider::orientationHint(const QString& path)
{
    ItemInfo info = ItemInfo::fromLocalFile(path);

    return info.orientation();
}

QSize DatabaseLoadSaveFileInfoProvider::dimensionsHint(const QString& path)
{
    ItemInfo info = ItemInfo::fromLocalFile(path);

    return info.dimensions();
}

} // namespace Digikam
