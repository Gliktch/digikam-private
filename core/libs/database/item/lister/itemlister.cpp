/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju    <renchi dot raju at gmail dot com>
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

ItemLister::ItemLister()
    : d(new Private)
{
}

ItemLister::~ItemLister()
{
    delete d;
}

void ItemLister::setRecursive(bool recursive)
{
    d->recursive = recursive;
}

void ItemLister::setListOnlyAvailable(bool listOnlyAvailable)
{
    d->listOnlyAvailableImages = listOnlyAvailable;
}

void ItemLister::list(ItemListerReceiver* const receiver,
                      const CoreDbUrl& url)
{
    if      (url.isAlbumUrl())
    {
        int albumRootId = url.albumRootId();
        QString album   = url.album();

        listPAlbum(receiver, albumRootId, album);
    }
    else if (url.isTagUrl())
    {
        listTag(receiver, url.tagIds());
    }
    else if (url.isDateUrl())
    {
        listDateRange(receiver, url.startDate(), url.endDate());
    }
    else if (url.isMapImagesUrl())
    {
        double lat1, lat2, lon1, lon2;
        url.areaCoordinates(&lat1, &lat2, &lon1, &lon2);
        listAreaRange(receiver, lat1, lat2, lon1, lon2);
    }
}

void ItemLister::listDateRange(ItemListerReceiver* const receiver,
                               const QDate& startDate,
                               const QDate& endDate)
{
    QList<QVariant> values;

    {
        CoreDbAccess access;
        access.backend()->execSql(QString::fromUtf8("SELECT DISTINCT Images.id, Images.name, Images.album, "
                                          "       Albums.albumRoot, "
                                          "       ImageInformation.rating, Images.category, "
                                          "       ImageInformation.format, ImageInformation.creationDate, "
                                          "       Images.modificationDate, Images.fileSize, "
                                          "       ImageInformation.width, ImageInformation.height "
                                          " FROM Images "
                                          "       LEFT JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                          "       INNER JOIN Albums ON Albums.id=Images.album "
                                          " WHERE Images.status=1 "
                                          "   AND ImageInformation.creationDate < ? "
                                          "   AND ImageInformation.creationDate >= ? "
                                          " ORDER BY Images.album;"),
                                  startOfDay(endDate),
                                  startOfDay(startDate),
                                  &values);
    }

    QSet<int> albumRoots = albumRootsToList();
    int       width      = 0;
    int       height     = 0;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        ItemListerRecord record;

        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
        ++it;
        record.albumRootID       = (*it).toInt();
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

        if (d->listOnlyAvailableImages && !albumRoots.contains(record.albumRootID))
        {
            continue;
        }

        record.imageSize         = QSize(width, height);

        receiver->receive(record);
    }
}

} // namespace Digikam
