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

SearchModel::SearchModel(QObject* const parent)
    : AbstractCheckableAlbumModel(Album::SEARCH,
                                  AlbumManager::instance()->findSAlbum(0),
                                  IgnoreRootAlbum, parent)
{
    m_columnHeader = i18n("Searches");

    setShowCount(false);

    // handle search icons

    albumSettingsChanged();

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(albumSettingsChanged()));
}

SAlbum* SearchModel::salbumForIndex(const QModelIndex& index) const
{
    return static_cast<SAlbum*>(AbstractCheckableAlbumModel::albumForIndex(index));
}

void SearchModel::setReplaceNames(const QHash<QString, QString>& replaceNames)
{
    m_replaceNames = replaceNames;
}

void SearchModel::addReplaceName(const QString& technicalName, const QString& userVisibleName)
{
    m_replaceNames.insert(technicalName, userVisibleName);
}

void SearchModel::setPixmapForNormalSearches(const QPixmap& pix)
{
    m_pixmaps.insert(-1, pix);
}

void SearchModel::setDefaultPixmap(const QPixmap& pix)
{
    m_pixmaps.insert(-2, pix);
}

void SearchModel::setPixmapForTimelineSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::TimeLineSearch, pix);
}

void SearchModel::setPixmapForHaarSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::HaarSearch, pix);
}

void SearchModel::setPixmapForMapSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::MapSearch, pix);
}

void SearchModel::setPixmapForDuplicatesSearches(const QPixmap& pix)
{
    m_pixmaps.insert(DatabaseSearch::DuplicatesSearch, pix);
}

QVariant SearchModel::albumData(Album* a, int role) const
{
    if      ((role == Qt::DisplayRole) || (role == AlbumTitleRole) || (role == Qt::ToolTipRole))
    {
        const SAlbum* const salbum = static_cast<SAlbum*>(a);
        QString title              = a->title();
        QString displayTitle       = salbum->displayTitle();

        return m_replaceNames.value(title, displayTitle);
    }
    else if (role == Qt::DecorationRole)
    {
        const SAlbum* const salbum = static_cast<SAlbum*>(a);
        QPixmap pixmap             = m_pixmaps.value(salbum->searchType());

        if (pixmap.isNull() && salbum->isNormalSearch())
        {
            pixmap = m_pixmaps.value(-1);
        }

        if (pixmap.isNull())
        {
            pixmap = m_pixmaps.value(-2);
        }

        return pixmap;
    }

    return AbstractCheckableAlbumModel::albumData(a, role);
}

Album* SearchModel::albumForId(int id) const
{
    return AlbumManager::instance()->findSAlbum(id);
}

void SearchModel::albumSettingsChanged()
{
    setPixmapForMapSearches(QIcon::fromTheme(QLatin1String("globe")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()));
    setPixmapForHaarSearches(QIcon::fromTheme(QLatin1String("tools-wizard")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()));
    setPixmapForNormalSearches(QIcon::fromTheme(QLatin1String("edit-find")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()));
    setPixmapForTimelineSearches(QIcon::fromTheme(QLatin1String("chronometer")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()));
}

} // namespace Digikam
