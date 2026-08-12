/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - private containers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummanager_p.h"

namespace Digikam
{

PAlbumPath::PAlbumPath(int albumRootId, const QString& albumPath)
    : albumRootId(albumRootId),
      albumPath  (albumPath)
{
}

PAlbumPath::PAlbumPath(const PAlbum* const album)
{
    if (album->isRoot())
    {
        albumRootId = -1;
    }
    else
    {
        albumRootId = album->albumRootId();
        albumPath   = album->albumPath();
    }
}

bool PAlbumPath::operator==(const PAlbumPath& other) const
{
    return (
            (other.albumRootId == albumRootId) &&
            (other.albumPath   == albumPath)
           );
}

// -----------------------------------------------------------------------------------

QString AlbumManager::Private::labelForAlbumRootAlbum(const CollectionLocation& location)
{
    QString label = location.label();

    if (label.isEmpty())
    {
        label = location.albumRootPath();
    }

    return label;
}

// -----------------------------------------------------------------------------------

ChangingDB::ChangingDB(AlbumManager::Private* const dd)
    : d(dd)
{
    d->changingDB = true;
}

ChangingDB::~ChangingDB()
{
    d->changingDB = false;
}

} // namespace Digikam
