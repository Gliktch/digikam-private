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

CheckableAlbumFilterModel::CheckableAlbumFilterModel(QObject* const parent)
    : AlbumFilterModel(parent)
{
}

void CheckableAlbumFilterModel::setSourceCheckableAlbumModel(AbstractCheckableAlbumModel* const source)
{
    AlbumFilterModel::setSourceAlbumModel(source);
}

void CheckableAlbumFilterModel::setSourceFilterModel(CheckableAlbumFilterModel* const source)
{
    AlbumFilterModel::setSourceFilterModel(source);
}

AbstractCheckableAlbumModel* CheckableAlbumFilterModel::sourceCheckableAlbumModel() const
{
    return (static_cast<AbstractCheckableAlbumModel*>(AlbumFilterModel::sourceAlbumModel()));
}

void CheckableAlbumFilterModel::setFilterChecked(bool filter)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_filterChecked = filter;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

void CheckableAlbumFilterModel::setFilterPartiallyChecked(bool filter)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_filterPartiallyChecked = filter;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

bool CheckableAlbumFilterModel::isFiltering() const
{
    return (AlbumFilterModel::isFiltering() || m_filterChecked || m_filterPartiallyChecked);
}

bool CheckableAlbumFilterModel::matches(Album* album) const
{
    bool accepted = AlbumFilterModel::matches(album);

    if (!m_filterChecked && !m_filterPartiallyChecked)
    {
        return accepted;
    }

    Qt::CheckState state = sourceCheckableAlbumModel()->checkState(album);

    bool stateAccepted = false;

    if (m_filterPartiallyChecked)
    {
        stateAccepted |= (state == Qt::PartiallyChecked);
    }

    if (m_filterChecked)
    {
        stateAccepted |= (state == Qt::Checked);
    }

    return (accepted && stateAccepted);
}

} // namespace Digikam
