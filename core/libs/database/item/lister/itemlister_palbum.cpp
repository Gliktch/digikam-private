/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database - PAlbum helpers.
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer  <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018      by Mario Frank    <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemlister_p.h"

namespace Digikam
{

void ItemLister::listPAlbum(ItemListerReceiver* const receiver,
                            int albumRootId,
                            const QString& album)
{
    if (d->listOnlyAvailableImages)
    {
        if (!CollectionManager::instance()->locationForAlbumRootId(albumRootId).isAvailable())
        {
            return;
        }
    }

    QList<QVariant> albumIds;

    if (d->recursive)
    {
        QList<int> intAlbumIds = CoreDbAccess().db()->getAlbumAndSubalbumsForPath(albumRootId, album);

        if (intAlbumIds.isEmpty())
        {
            return;
        }

        for (int id : std::as_const(intAlbumIds))
        {
            albumIds << id;
        }
    }
    else
    {
        int albumId = CoreDbAccess().db()->getAlbumForPath(albumRootId, album, false);

        if (albumId == -1)
        {
            return;
        }

        albumIds << albumId;
    }

    QList<QVariant> values;

    QString query = QString::fromUtf8("SELECT DISTINCT Images.id, Images.name, Images.album, "
                    "       ImageInformation.rating, Images.category, "
                    "       ImageInformation.format, ImageInformation.creationDate, "
                    "       Images.modificationDate, Images.fileSize, "
                    "       ImageInformation.width, ImageInformation.height "
                    " FROM Images "
                    "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                    " WHERE Images.status=1 AND ");

    if (d->recursive)
    {
        // SQLite allows no more than 999 parameters

        const int maxParams = CoreDbAccess().backend()->maximumBoundValues();

        for (int i = 0 ; i < albumIds.size() ; ++i)
        {
            QString q           = query;
            QList<QVariant> ids =  (albumIds.size() <= maxParams) ? albumIds : albumIds.mid(i, maxParams);
            i                  += ids.count();

            QList<QVariant> v;
            CoreDbAccess  access;
            q += QString::fromUtf8("Images.album IN (");
            access.db()->addBoundValuePlaceholders(q, ids.size());
            q += QString::fromUtf8(");");
            access.backend()->execSql(q, ids, &v);

            values += v;
        }
    }
    else
    {
        CoreDbAccess access;
        query += QString::fromUtf8("Images.album = ?;");
        access.backend()->execSql(query, albumIds, &values);
    }

    int width  = 0;
    int height = 0;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        ItemListerRecord record;
        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
        ++it;
        record.rating            = (*it).toInt();
        ++it;
        record.category          = (DatabaseItem::Category)(*it).toInt();
        ++it;
        record.format            = (*it).toString();
        ++it;
        record.creationDate      = asDateTimeUTC((*it).toDateTime());
        ++it;
        record.modificationDate  = asDateTimeUTC((*it).toDateTime());
        ++it;
        record.fileSize          = (*it).toLongLong();
        ++it;
        width                    = (*it).toInt();
        ++it;
        height                   = (*it).toInt();
        ++it;

        record.imageSize         = QSize(width, height);

        record.albumRootID = albumRootId;

        receiver->receive(record);
    }
}

QSet<int> ItemLister::albumRootsToList() const
{
    if (!d->listOnlyAvailableImages)
    {
        return QSet<int>();    // invalid value, all album roots shall be listed
    }

    QList<CollectionLocation> locations = CollectionManager::instance()->allAvailableLocations();
    QSet<int>                 ids;

    for (const CollectionLocation& location : std::as_const(locations))
    {
        ids << location.id();
    }

    return ids;
}

} // namespace Digikam
