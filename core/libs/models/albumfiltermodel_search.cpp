/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumfiltermodel_p.h"

namespace Digikam
{

SearchFilterModel::SearchFilterModel(QObject* const parent)
    : CheckableAlbumFilterModel(parent)
{
}

void SearchFilterModel::setSourceSearchModel(SearchModel* const source)
{
    setSourceAlbumModel(source);
}

SearchModel* SearchFilterModel::sourceSearchModel() const
{
    return (dynamic_cast<SearchModel*> (sourceModel()));
}

void SearchFilterModel::setFilterSearchType(DatabaseSearch::Type type)
{
    setTypeFilter(type);
}

void SearchFilterModel::listNormalSearches()
{
    setTypeFilter(-1);
}

void SearchFilterModel::listAllSearches()
{
    setTypeFilter(-2);
}

void SearchFilterModel::listTimelineSearches()
{
    setTypeFilter(DatabaseSearch::TimeLineSearch);
}

void SearchFilterModel::listHaarSearches()
{
    setTypeFilter(DatabaseSearch::HaarSearch);
}

void SearchFilterModel::listMapSearches()
{
    setTypeFilter(DatabaseSearch::MapSearch);
}

void SearchFilterModel::listDuplicatesSearches()
{
    setTypeFilter(DatabaseSearch::DuplicatesSearch);
}

void SearchFilterModel::setTypeFilter(int type)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_searchType = type;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

void SearchFilterModel::setListTemporarySearches(bool list)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_listTemporary = list;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

bool SearchFilterModel::isFiltering() const
{
    return ((m_searchType != -2) || !m_listTemporary);
}

bool SearchFilterModel::matches(Album* album) const
{
    if (!CheckableAlbumFilterModel::matches(album))
    {
        return false;
    }

    const SAlbum* const salbum = static_cast<SAlbum*>(album);

    if      (m_searchType == -1)
    {
        if (!salbum->isNormalSearch())
        {
            return false;
        }
    }
    else if (m_searchType == -2)
    {
    }
    else
    {
        if (salbum->searchType() != (DatabaseSearch::Type)m_searchType)
        {
            return false;
        }
    }

    if (!m_listTemporary && salbum->isTemporarySearch())
    {
        return false;
    }

    return true;
}

void SearchFilterModel::setSourceSearchAlbumModel(AbstractAlbumModel* const source)
{
    AlbumFilterModel::setSourceAlbumModel(source);
}

} // namespace Digikam
