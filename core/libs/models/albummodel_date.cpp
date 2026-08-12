/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummodel_p.h"

namespace Digikam
{

DateAlbumModel::DateAlbumModel(QObject* const parent)
    : AbstractCountingAlbumModel(Album::DATE,
                                 AlbumManager::instance()->findDAlbum(0),
                                 IgnoreRootAlbum, parent)
{
    m_columnHeader = i18n("Dates");

    connect(AlbumManager::instance(), SIGNAL(signalDAlbumsDirty(QMap<YearMonth,int>)),
            this, SLOT(setYearMonthMap(QMap<YearMonth,int>)));

    setYearMonthMap(AlbumManager::instance()->getDAlbumsCount());

    setup();
}

DAlbum* DateAlbumModel::dalbumForIndex(const QModelIndex& index) const
{
    return (static_cast<DAlbum*>(AbstractCountingAlbumModel::albumForIndex(index)));
}

QModelIndex DateAlbumModel::monthIndexForDate(const QDate& date) const
{
    // iterate over all years

    for (int yearIndex = 0 ; yearIndex < rowCount() ; ++yearIndex)
    {
        QModelIndex year              = index(yearIndex, 0);
        const DAlbum* const yearAlbum = dalbumForIndex(year);

        // do not search through months if we are sure, that the year already
        // does not match

        if (
            yearAlbum                            &&
            (yearAlbum->range() == DAlbum::Year) &&
            (yearAlbum->date().year() != date.year())
           )
        {
            continue;
        }

        // search the album with the correct month

        for (int monthIndex = 0 ; monthIndex < rowCount(year) ; ++monthIndex)
        {
            QModelIndex month              = index(monthIndex, 0, year);
            const DAlbum* const monthAlbum = dalbumForIndex(month);

            if (
                monthAlbum                                  &&
                (monthAlbum->range() == DAlbum::Month)      &&
                (monthAlbum->date().year() == date.year())  &&
                (monthAlbum->date().month() == date.month())
               )
            {
                return month;
            }
        }

    }

    return QModelIndex();
}

void DateAlbumModel::setPixmaps(const QPixmap& forYearAlbums, const QPixmap& forMonthAlbums)
{
    m_yearPixmap  = forYearAlbums;
    m_monthPixmap = forMonthAlbums;
}

QString DateAlbumModel::albumName(Album* album) const
{
    const DAlbum* const dalbum = static_cast<DAlbum*>(album);

    if (dalbum->range() == DAlbum::Year)
    {
        return QString::number(dalbum->date().year());
    }
    else
    {
        return QLocale().standaloneMonthName(dalbum->date().month(), QLocale::LongFormat);
    }
}

QVariant DateAlbumModel::decorationRoleData(Album* album) const
{
    const DAlbum* const dalbum = static_cast<DAlbum*>(album);

    if (dalbum->range() == DAlbum::Year)
    {
        return m_yearPixmap;
    }
    else
    {
        return m_monthPixmap;
    }
}

QVariant DateAlbumModel::sortRoleData(Album* a) const
{
    const DAlbum* const dalbum = static_cast<DAlbum*>(a);

    if (dalbum)
    {
        return dalbum->date();
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "There must be a data album.";

    return QDate();
}

Album* DateAlbumModel::albumForId(int id) const
{
    return AlbumManager::instance()->findDAlbum(id);
}

void DateAlbumModel::setYearMonthMap(const QMap<YearMonth, int>& yearMonthMap)
{
    AlbumIterator it(rootAlbum());

    QHash<int, int> albumToCountHash;

    while (it.current())
    {
        const DAlbum* const dalbum = static_cast<DAlbum*>(*it);
        QDate date                 = dalbum->date();

        switch (dalbum->range())
        {
            case DAlbum::Month:
            {
                QMap<YearMonth, int>::const_iterator it2 = yearMonthMap.constFind(YearMonth(date.year(), date.month()));

                if (it2 != yearMonthMap.constEnd())
                {
                    albumToCountHash.insert((*it)->id(), it2.value());
                }

                break;
            }

            case DAlbum::Year:
            {
                // a year itself cannot contain images and therefore always has count 0

                albumToCountHash.insert((*it)->id(), 0);
                break;
            }

            default:
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Untreated DAlbum range " << dalbum->range();
                albumToCountHash.insert((*it)->id(), 0);
                break;
            }
        }

        ++it;
    }

    setCountHash(albumToCountHash);
}

} // namespace Digikam
