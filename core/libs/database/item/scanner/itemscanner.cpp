/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item.
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemscanner_p.h"

namespace Digikam
{

ItemScanner::ItemScanner(const QFileInfo& info, const ItemScanInfo& scanInfo)
    : d(new Private)
{
    d->fileInfo = info;
    d->scanInfo = scanInfo;
}

ItemScanner::ItemScanner(const QFileInfo& info)
    : d(new Private)
{
    d->fileInfo = info;
}

ItemScanner::ItemScanner(qlonglong imageid)
    : d(new Private)
{
    ItemShortInfo shortInfo;
    {
        CoreDbAccess access;
        shortInfo   = access.db()->getItemShortInfo(imageid);
        d->scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootID);
    d->fileInfo           = QFileInfo(CoreDbUrl::fromAlbumAndName(shortInfo.itemName,
                                      shortInfo.album, QUrl::fromLocalFile(albumRootPath),
                                      shortInfo.albumRootID).fileUrl().toLocalFile());
}

ItemScanner::~ItemScanner()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Finishing took" << d->timer.elapsed() << "ms";
    delete d;
}

void ItemScanner::setCategory(DatabaseItem::Category category)
{
    // we don't have the necessary information in this class, but in CollectionScanner

    d->scanInfo.category = category;
}

const ItemScanInfo& ItemScanner::itemScanInfo() const
{
    return d->scanInfo;
}

bool ItemScanner::lessThanForIdentity(const ItemScanInfo& a, const ItemScanInfo& b)
{
    if (a.status != b.status)
    {
        // First: sort by status

        // put UndefinedStatus to back

        if (a.status == DatabaseItem::UndefinedStatus)
        {
            return false;
        }

        // enum values are in the order we want it

        return a.status < b.status;
    }
    else
    {
        // Second: sort by modification date, descending

        return a.modificationDate > b.modificationDate;
    }
}

bool ItemScanner::hasValidField(const QVariantList& list)
{
    for (QVariantList::const_iterator it = list.constBegin() ;
         it != list.constEnd() ; ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
}

void ItemScanner::sortByProximity(QList<ItemInfo>& list, const ItemInfo& subject)
{
    if (!list.isEmpty() && !subject.isNull())
    {
        std::stable_sort(list.begin(), list.end(), LessThanByProximityToSubject(subject));
    }
}

void ItemScanner::loadFromDisk()
{
    if (d->loadedFromDisk)
    {
        return;
    }

    d->loadedFromDisk = true;
    d->metadata->registerMetadataSettings();
    d->hasMetadata    = d->metadata->load(d->fileInfo.filePath(), true);

    if (d->scanInfo.category == DatabaseItem::Image)
    {
        d->hasImage = d->img.loadItemInfo(d->fileInfo.filePath(), false, false, false, false);
    }
    else
    {
        d->hasImage = false;
    }

    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();
    QDateTime modificationDate           = asDateTimeUTC(d->fileInfo.lastModified());

    if (
        settings.useXMPSidecar4Reading              &&
        DMetadata::hasSidecar(d->fileInfo.filePath())
       )
    {
        QString filePath      = DMetadata::sidecarPath(d->fileInfo.filePath());
        QDateTime sidecarDate = asDateTimeUTC(QFileInfo(filePath).lastModified());

        if (sidecarDate > modificationDate)
        {
            modificationDate = sidecarDate;
        }
    }

    d->scanInfo.itemName         = d->fileInfo.fileName();
    d->scanInfo.fileSize         = d->fileInfo.size();
    d->scanInfo.modificationDate = modificationDate;

    // category is set by setCategory
    // NOTE: call uniqueHash after loading the image above, else it will fail

    d->scanInfo.uniqueHash       = uniqueHash();

    // faster than loading twice from disk

    if (d->hasMetadata)
    {
        d->img.setMetadata(d->metadata->data());
    }
}

QString ItemScanner::formatToString(const QString& format)
{
    // image -------------------------------------------------------------------

    if      (format == QLatin1String("JPG"))
    {
        return QLatin1String("JPEG");
    }
    else if (format == QLatin1String("PNG"))
    {
        return format;
    }
    else if (format == QLatin1String("TIFF"))
    {
        return format;
    }
    else if (format == QLatin1String("PPM"))
    {
        return format;
    }
    else if ((format == QLatin1String("JP2")) || (format == QLatin1String("JP2k")) || (format == QLatin1String("JP2K")))
    {
        return QLatin1String("JPEG 2000");
    }
    else if (format.startsWith(QLatin1String("RAW-")))
    {
        return i18nc("RAW image file (), the parentheses contain the file suffix, like MRW",
                     "RAW image file (%1)",
                     format.mid(4));
    }

    // video -------------------------------------------------------------------

    else if (format == QLatin1String("MPEG"))
    {
        return format;
    }
    else if (format == QLatin1String("AVI"))
    {
        return format;
    }
    else if (format == QLatin1String("MOV"))
    {
        return QLatin1String("Quicktime");
    }
    else if (format == QLatin1String("WMF"))
    {
        return QLatin1String("Windows MetaFile");
    }
    else if (format == QLatin1String("WMV"))
    {
        return QLatin1String("Windows Media Video");
    }
    else if (format == QLatin1String("MP4"))
    {
        return QLatin1String("MPEG-4");
    }
    else if (format == QLatin1String("3GP"))
    {
        return QLatin1String("3GPP");
    }

    // audio -------------------------------------------------------------------

    else if (format == QLatin1String("OGG"))
    {
        return QLatin1String("Ogg");
    }
    else if (format == QLatin1String("MP3"))
    {
        return format;
    }
    else if (format == QLatin1String("WMA"))
    {
        return QLatin1String("Windows Media Audio");
    }
    else if (format == QLatin1String("WAV"))
    {
        return QLatin1String("WAVE");
    }
    else
    {
        return format;
    }
}

} // namespace Digikam
