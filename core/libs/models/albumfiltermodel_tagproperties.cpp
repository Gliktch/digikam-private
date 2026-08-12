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

TagPropertiesFilterModel::TagPropertiesFilterModel(QObject* const parent)
    : CheckableAlbumFilterModel(parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalTagPropertiesChanged(TAlbum*)),
            this, SLOT(tagPropertiesChanged(TAlbum*)));
}

void TagPropertiesFilterModel::setSourceAlbumModel(TagModel* const source)
{
    CheckableAlbumFilterModel::setSourceAlbumModel(source);
}

TagModel* TagPropertiesFilterModel::sourceTagModel() const
{
    return (dynamic_cast<TagModel*>(sourceModel()));
}

void TagPropertiesFilterModel::listOnlyTagsWithProperty(const QString& property)
{
    if (m_propertiesWhiteList.contains(property))
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_propertiesWhiteList << property;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

void TagPropertiesFilterModel::removeListOnlyProperty(const QString& property)
{
    if (!m_propertiesWhiteList.contains(property))
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_propertiesWhiteList.remove(property);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

void TagPropertiesFilterModel::doNotListTagsWithProperty(const QString& property)
{
    if (m_propertiesBlackList.contains(property))
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_propertiesBlackList << property;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

void TagPropertiesFilterModel::removeDoNotListProperty(const QString& property)
{
    if (!m_propertiesBlackList.contains(property))
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    beginFilterChange();

#endif

    m_propertiesBlackList.remove(property);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

    endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

    invalidateFilter();

#endif

    Q_EMIT signalFilterChanged();
}

bool TagPropertiesFilterModel::isFiltering() const
{
    return (!m_propertiesWhiteList.isEmpty() || !m_propertiesBlackList.isEmpty());
}

void TagPropertiesFilterModel::tagPropertiesChanged(TAlbum*)
{
    // I do not expect batch changes. Otherwise we'll need a timer.

    if (this->isFiltering())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))

        beginFilterChange();
        endFilterChange(QSortFilterProxyModel::Direction::Rows);

#else

        invalidateFilter();

#endif

        // Sort new when tag properties change.

        invalidate();
    }
}

bool TagPropertiesFilterModel::matches(Album* album) const
{
    if (!CheckableAlbumFilterModel::matches(album))
    {
        return false;
    }

    const TAlbum* const talbum = static_cast<TAlbum*>(album);

    for (const QString& prop : std::as_const(m_propertiesBlackList))
    {
        if (talbum->hasProperty(prop))
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    for (const QString& prop : std::as_const(m_propertiesWhiteList))
    {
        if (!talbum->hasProperty(prop))
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    return true;
}

} // namespace Digikam
