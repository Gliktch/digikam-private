/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applicationsettings_p.h"

namespace Digikam
{

void ApplicationSettings::setTreeViewIconSize(int val)
{
    d->treeThumbnailSize = val;
}

int ApplicationSettings::getTreeViewIconSize() const
{
    return ((d->treeThumbnailSize < 8) ||
            (d->treeThumbnailSize > 64)) ? 32 : d->treeThumbnailSize;
}

void ApplicationSettings::setTreeViewFaceSize(int val)
{
    d->treeThumbFaceSize = val;
}

int ApplicationSettings::getTreeViewFaceSize() const
{
    return ((d->treeThumbFaceSize < 8) ||
            (d->treeThumbFaceSize > 128)) ? 48 : d->treeThumbFaceSize;
}

void ApplicationSettings::setTreeViewFont(const QFont& font)
{
    d->treeviewFont = font;
}

QFont ApplicationSettings::getTreeViewFont() const
{
    return d->treeviewFont;
}

void ApplicationSettings::setAlbumSortRole(const ApplicationSettings::AlbumSortRole role)
{
    d->albumSortRole = role;
}

ApplicationSettings::AlbumSortRole ApplicationSettings::getAlbumSortRole() const
{
    return d->albumSortRole;
}

void ApplicationSettings::setAlbumMonitoring(bool val)
{
    d->albumMonitoring = val;
}

bool ApplicationSettings::getAlbumMonitoring() const
{
    return d->albumMonitoring;
}

void ApplicationSettings::setRecurseAlbums(bool val)
{
    d->recursiveAlbums = val;

    Q_EMIT recurseSettingsChanged();
}

bool ApplicationSettings::getRecurseAlbums() const
{
    return d->recursiveAlbums;
}

void ApplicationSettings::setRecurseTags(bool val)
{
    d->recursiveTags = val;

    Q_EMIT recurseSettingsChanged();
}

bool ApplicationSettings::getRecurseTags() const
{
    return d->recursiveTags;
}

void ApplicationSettings::setAllGroupsOpen(bool val)
{
    d->allGroupsOpen = val;
}

bool ApplicationSettings::getAllGroupsOpen() const
{
    return d->allGroupsOpen;
}

void ApplicationSettings::setShowFolderTreeViewItemsCount(bool val)
{
    d->showFolderTreeViewItemsCount = val;
}

bool ApplicationSettings::getShowFolderTreeViewItemsCount() const
{
    return d->showFolderTreeViewItemsCount;
}

void ApplicationSettings::setAlbumCategoryNames(const QStringList& list)
{
    d->albumCategoryNames = list;
}

QStringList ApplicationSettings::getAlbumCategoryNames() const
{
    return d->albumCategoryNames;
}

bool ApplicationSettings::addAlbumCategoryName(const QString& name) const
{
    if (d->albumCategoryNames.contains(name))
    {
        return false;
    }

    d->albumCategoryNames.append(name);

    return true;
}

bool ApplicationSettings::delAlbumCategoryName(const QString& name) const
{
    uint count = d->albumCategoryNames.removeAll(name);

    return ((count > 0) ? true : false);
}

} // namespace Digikam
