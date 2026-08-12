/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management - location helpers.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionmanager_p.h"

namespace Digikam
{

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    QReadLocker readLocker(&d->lock);

    QStringList list;

    for (const AlbumRootLocation* const location : std::as_const(d->locations))
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            list << location->albumRootPath();
        }
    }

    return list;
}

QString CollectionManager::albumRootPath(int id)
{
    QReadLocker readLocker(&d->lock);

    const CollectionLocation* const location = d->locations.value(id);

    if (location && location->status() == CollectionLocation::LocationAvailable)
    {
        return location->albumRootPath();
    }

    return QString();
}

QString CollectionManager::albumRootLabel(int id)
{
    QReadLocker readLocker(&d->lock);

    const CollectionLocation* const location = d->locations.value(id);

    if (location && location->status() == CollectionLocation::LocationAvailable)
    {
        return location->label();
    }

    return QString();
}

QUrl CollectionManager::albumRoot(const QUrl& fileUrl)
{
    return QUrl::fromLocalFile(albumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile()));
}

QString CollectionManager::albumRootPath(const QUrl& fileUrl)
{
    return albumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::albumRootPath(const QString& givenPath)
{
    QReadLocker readLocker(&d->lock);

    for (const AlbumRootLocation* const location : std::as_const(d->locations))
    {
        QString rootPath = location->albumRootPath();
        QString filePath = QDir::fromNativeSeparators(givenPath);

        if (!rootPath.isEmpty() && filePath.startsWith(rootPath))
        {
            // see also bug #221155 for extra checks

            if ((filePath == rootPath) || filePath.startsWith(rootPath + QLatin1Char('/')))
            {
                return location->albumRootPath();
            }
        }
    }

    return QString();
}

bool CollectionManager::isAlbumRoot(const QUrl& fileUrl)
{
    return isAlbumRoot(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

bool CollectionManager::isAlbumRoot(const QString& filePath)
{
    QReadLocker readLocker(&d->lock);

    for (const AlbumRootLocation* const location : std::as_const(d->locations))
    {
        if (filePath == location->albumRootPath())
        {   // cppcheck-suppress useStlAlgorithm
            return true;
        }
    }

    return false;
}

QString CollectionManager::album(const QUrl& fileUrl)
{
    return album(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::album(const QString& filePath)
{
    QReadLocker readLocker(&d->lock);

    for (const AlbumRootLocation* const location : std::as_const(d->locations))
    {
        QString rootPath = location->albumRootPath();

        if (rootPath.isEmpty() || !filePath.startsWith(rootPath))
        {
            continue;
        }

        QString albm = filePath.mid(rootPath.length());

        if      (albm.isEmpty() || (albm == QLatin1String("/")))
        {
            return QLatin1String("/");
        }
        else if (albm.startsWith(QLatin1Char('/')))
        {
            if (albm.endsWith(QLatin1Char('/')))
            {
                albm.chop(1);
            }

            return albm;
        }
    }

    return QString();
}

QString CollectionManager::album(const CollectionLocation& location, const QUrl& fileUrl)
{
    return album(location, fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::album(const CollectionLocation& location, const QString& filePath)
{
    if (location.isNull())
    {
        return QString();
    }

    QString absolutePath = location.albumRootPath();

    if (filePath == absolutePath)
    {
        return QLatin1String("/");
    }
    else
    {
        QString albm = filePath.mid(absolutePath.length());

        if (albm.endsWith(QLatin1Char('/')))
        {
            albm.chop(1);
        }

        return albm;
    }
}

QUrl CollectionManager::oneAlbumRoot()
{
    return QUrl::fromLocalFile(oneAlbumRootPath());
}

QString CollectionManager::oneAlbumRootPath()
{
    QReadLocker readLocker(&d->lock);

    for (const AlbumRootLocation* const location : std::as_const(d->locations))
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {   // cppcheck-suppress useStlAlgorithm
            return location->albumRootPath();
        }
    }

    return QString();
}

void CollectionManager::slotAlbumRootChange(const AlbumRootChangeset& changeset)
{
    if (d->changingDB)
    {
        return;
    }

    switch (changeset.operation())
    {
        case AlbumRootChangeset::Added:
        case AlbumRootChangeset::Deleted:
        {
            updateLocations();
            break;
        }

        case AlbumRootChangeset::PropertiesChanged:
        {
            // label has changed

            CollectionLocation toBeEmitted;

            {
                QReadLocker readLocker(&d->lock);
                AlbumRootLocation* const location = d->locations.value(changeset.albumRootId());

                if (location)
                {
                    QList<AlbumRootInfo> infos = CoreDbAccess().db()->getAlbumRoots();

                    for (const AlbumRootInfo& info : std::as_const(infos))
                    {
                        if (info.id == location->id())
                        {    // cppcheck-suppress useStlAlgorithm
                            location->setLabel(info.label);
                            toBeEmitted = *location;
                            break;
                        }
                    }
                }
            }

            if (!toBeEmitted.isNull())
            {
                Q_EMIT locationPropertiesChanged(toBeEmitted);
            }

            break;
        }

        case AlbumRootChangeset::Unknown:
        {
            break;
        }
    }
}

} // namespace Digikam
