/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-14
 * Description : Load and cache tag thumbnails
 *
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumthumbnailloader.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>
#include <QMap>
#include <QCache>
#include <QPair>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "thememanager.h"
#include "applicationsettings.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "iteminfo.h"
#include "faceutils.h"
#include "metaenginesettings.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "thumbnailcreator.h"
#include "dio.h"

namespace Digikam
{

typedef QMap<QPair<qlonglong, QString>, QList<int> > IdAlbumMap;
typedef QMap<int, QPixmap>                           AlbumThumbnailMap;

class Q_DECL_HIDDEN AlbumThumbnailLoaderCreator
{
public:

    AlbumThumbnailLoader object;
};

Q_GLOBAL_STATIC(AlbumThumbnailLoaderCreator, creator)

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumThumbnailLoader::Private
{
public:

    Private() = default;

public:

    int                                  iconSize               = ApplicationSettings::instance()->getTreeViewIconSize();
    int                                  faceSize               = ApplicationSettings::instance()->getTreeViewFaceSize();
    int                                  minBlendSize           = 20;

    ThumbnailLoadThread*                 iconTagThumbThread     = nullptr;
    ThumbnailLoadThread*                 iconFaceThumbThread    = nullptr;
    ThumbnailLoadThread*                 iconAlbumThumbThread   = nullptr;

    QIcon                                fallBackIcon           = QIcon::fromTheme(QLatin1String("dialog-cancel"));

    IdAlbumMap                           idAlbumMap;

    AlbumThumbnailMap                    thumbnailMap;

    QCache<QPair<QString, int>, QPixmap> iconCache;
};

bool operator<(const ThumbnailIdentifier& a, const ThumbnailIdentifier& b)
{
    if (a.id || b.id)
    {
        return (a.id < b.id);
    }
    else
    {
        return (a.filePath < b.filePath);
    }
}

AlbumThumbnailLoader* AlbumThumbnailLoader::instance()
{
    return &creator->object;
}

AlbumThumbnailLoader::~AlbumThumbnailLoader()
{
    delete d->iconAlbumThumbThread;
    delete d->iconFaceThumbThread;
    delete d->iconTagThumbThread;

    delete d;
}

AlbumThumbnailLoader::AlbumThumbnailLoader()
    : QObject(),
      d      (new Private)
{
    connect(this, SIGNAL(signalDispatchThumbnailInternal(int,QPixmap)),
            this, SLOT(slotDispatchThumbnailInternal(int,QPixmap)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotIconChanged(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotIconChanged(Album*)));

    connect(ThemeManager::instance(), &ThemeManager::signalThemeChanged,
            this, [this]()
        {
            d->iconCache.clear();
        }
    );
}

void AlbumThumbnailLoader::cleanUp()
{
    if (d->iconAlbumThumbThread)
    {
        d->iconAlbumThumbThread->stopAllTasks();
        d->iconAlbumThumbThread->wait();
    }

    if (d->iconFaceThumbThread)
    {
        d->iconFaceThumbThread->stopAllTasks();
        d->iconFaceThumbThread->wait();
    }

    if (d->iconTagThumbThread)
    {
        d->iconTagThumbThread->stopAllTasks();
        d->iconTagThumbThread->wait();
    }
}

QPixmap AlbumThumbnailLoader::getStandardTagIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("tag"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardTagRootIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("document-open"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardTagIcon(TAlbum* const album, RelativeSize relativeSize)
{
    if (album->isRoot())
    {
        return getStandardTagRootIcon(relativeSize);
    }
    else
    {
        return getStandardTagIcon(relativeSize);
    }
}

QPixmap AlbumThumbnailLoader::getStandardAlbumIcon(PAlbum* const album, RelativeSize relativeSize)
{
    if      (album->isRoot() || album->isAlbumRoot())
    {
        return getStandardAlbumRootIcon(relativeSize);
    }
    else if (album->isTrashAlbum())
    {
        const PAlbum* const palbum = static_cast<PAlbum*>(album->parent());

        if (palbum)
        {
            if (DIO::getTrashCounter(palbum->folderPath()) > 0)
            {
                return getStandardFullTrashIcon(relativeSize);
            }
        }

        return getStandardEmptyTrashIcon(relativeSize);
    }

    return getStandardAlbumIcon(relativeSize);
}

QPixmap AlbumThumbnailLoader::getStandardFaceIcon(TAlbum* const /*album*/, RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("smiley"), computeFaceSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getNewTagIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("tag-new"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardEmptyTrashIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("user-trash"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardFullTrashIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("user-trash-full"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardAlbumRootIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("folder-pictures"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardOfflineIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("data-error"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardAlbumIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("folder"), computeIconSize(relativeSize));
}

int AlbumThumbnailLoader::computeIconSize(RelativeSize relativeSize) const
{
    if (relativeSize == SmallerSize)
    {
        // when size was 32 smaller was 20. Scale.

        return lround(20.0 / 32.0 * (double)d->iconSize);
    }

    return d->iconSize;
}

int AlbumThumbnailLoader::computeFaceSize(RelativeSize relativeSize) const
{
    if (relativeSize == SmallerSize)
    {
        // when size was 32 smaller was 20. Scale.

        return lround(20.0 / 32.0 * (double)d->faceSize);
    }

    return d->faceSize;
}

QPixmap AlbumThumbnailLoader::loadIcon(const QString& name, int size) const
{
    const QPixmap* cachePix = d->iconCache[qMakePair(name, size)];

    if (!cachePix)
    {
        QPixmap pix;

        // We check for a slash to see if it's a path to a file.
        // This is significantly faster than check with QFileInfo.

        if (name.contains(QLatin1Char('/')))
        {
            if (pix.load(name))
            {
                pix = pix.scaled(size,
                                 size,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
            }
            else
            {
                pix = d->fallBackIcon.pixmap(size);
            }
        }
        else
        {
            pix = QIcon::fromTheme(name, d->fallBackIcon).pixmap(size);
        }

        d->iconCache.insert(qMakePair(name, size), new QPixmap(pix));

        return pix;
    }

    return (*cachePix); // ownership of the pointer is kept by the icon cache.
}

bool AlbumThumbnailLoader::getTagThumbnail(TAlbum* const album, QPixmap& icon)
{
    if      (album->iconId() && (d->iconSize > d->minBlendSize))
    {
        addUrl(album, album->iconId());
        icon = QPixmap();

        return true;
    }
    else if (!album->icon().isEmpty())
    {
        icon = loadIcon(album->icon(), d->iconSize);

        return false;
    }

    icon = QPixmap();

    return false;
}

QPixmap AlbumThumbnailLoader::getTagThumbnailDirectly(TAlbum* const album)
{
    if      (album->iconId() && (d->iconSize > d->minBlendSize))
    {
        // icon cached?

        AlbumThumbnailMap::const_iterator it = d->thumbnailMap.constFind(album->globalID());

        if (it != d->thumbnailMap.constEnd())
        {
            return *it;
        }

        addUrl(album, album->iconId());
    }
    else if (!album->icon().isEmpty())
    {
        QPixmap pixmap = loadIcon(album->icon(), d->iconSize);

        return pixmap;
    }

    return getStandardTagIcon(album);
}

QPixmap AlbumThumbnailLoader::getFaceThumbnailDirectly(TAlbum* const album)
{
    if      (album->iconId() && (d->faceSize > d->minBlendSize))
    {
        // icon cached?

        AlbumThumbnailMap::const_iterator it = d->thumbnailMap.constFind(album->globalID());

        if (it != d->thumbnailMap.constEnd())
        {
            return *it;
        }

        addUrl(album, album->iconId());
    }
    else if (!album->icon().isEmpty())
    {
        QPixmap pixmap = loadIcon(album->icon(), d->faceSize);

        return pixmap;
    }
    else if (!album->iconId() && !FaceTags::isSystemPersonTagId(album->id()))
    {
        qlonglong firstItemId = CoreDbAccess().db()->getFirstItemWithFaceTag(album->id());

        if (firstItemId > 0)
        {
            QString err;

            if (!AlbumManager::instance()->updateTAlbumIcon(album, QString(),
                                                            firstItemId, err))
            {
               qCDebug(DIGIKAM_GENERAL_LOG) << err;
            }
            else
            {
                addUrl(album, firstItemId);
            }
        }
    }

    return getStandardFaceIcon(album);
}

bool AlbumThumbnailLoader::getAlbumThumbnail(PAlbum* const album)
{
    if (album->iconId() && (d->iconSize > d->minBlendSize))
    {
        addUrl(album, album->iconId());
    }
    else
    {
        return false;
    }

    return true;
}

QPixmap AlbumThumbnailLoader::getAlbumThumbnailDirectly(PAlbum* const album)
{
    if (album->isAlbumRoot())
    {
        CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(album->albumRootId());

        if (location.isNull() || !location.isAvailable())
        {
            return getStandardOfflineIcon();
        }
    }

    if (album->iconId() && !album->isTrashAlbum() && (d->iconSize > d->minBlendSize))
    {
        // icon cached?

        AlbumThumbnailMap::const_iterator it = d->thumbnailMap.constFind(album->globalID());

        if (it != d->thumbnailMap.constEnd())
        {
            return *it;
        }

        // schedule for loading

        addUrl(album, album->iconId());
    }

    return getStandardAlbumIcon(album);
}

void AlbumThumbnailLoader::addUrl(Album* const album, qlonglong id)
{
    // First check cached thumbnails.
    // We use a private cache which is actually a map to be sure to cache _all_ album thumbnails.
    // At startup, this is not relevant, as the views will add their requests in a row.
    // This is to speed up context menu and IE imagedescedit

    AlbumThumbnailMap::const_iterator ttit = d->thumbnailMap.constFind(album->globalID());

    if (ttit != d->thumbnailMap.constEnd())
    {
        // It is not necessary to return cached icon asynchronously - they could be
        // returned by getTagThumbnail already - but this would make the API
        // less elegant, it feels much better this way.

        Q_EMIT signalDispatchThumbnailInternal(album->globalID(), *ttit);

        return;
    }

    // Finding face rect to identify correct tag

    QRect faceRect = QRect();

    if ((album->type() == Album::TAG) && static_cast<TAlbum*>(album)->hasProperty(TagPropertyName::person()))
    {
        QList<FaceTagsIface> faces = FaceTagsEditor().databaseFaces(id);

        for (const FaceTagsIface& face : std::as_const(faces))
        {
            if (face.tagId() == album->id())
            {
                faceRect = FaceUtils::faceRectToDisplayRect(face.region().toRect());
                break;
            }
        }
    }

    // Simple way to put QRect into QMap

    QString faceRectStr = QString(QLatin1String("%1%2%3%4"))
            .arg(faceRect.x()).arg(faceRect.y()).arg(faceRect.right()).arg(faceRect.bottom());

    // Check if the URL has already been added

    IdAlbumMap::iterator it = d->idAlbumMap.find(QPair<qlonglong, QString>(id, faceRectStr));

    if (it == d->idAlbumMap.end())
    {
        // use two threads so that tag and album thumbnails are loaded
        // in parallel and not first album, then tag thumbnails

        if (album->type() == Album::TAG)
        {
            if (!d->iconTagThumbThread)
            {
                d->iconTagThumbThread = new ThumbnailLoadThread();
                d->iconTagThumbThread->setThumbnailSize(d->iconSize);
                d->iconTagThumbThread->setSendSurrogatePixmap(false);

                connect(d->iconTagThumbThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                        this, SLOT(slotGotThumbnailFromIcon(LoadingDescription,QPixmap)),
                        Qt::QueuedConnection);
            }

            if (!d->iconFaceThumbThread)
            {
                d->iconFaceThumbThread = new ThumbnailLoadThread();
                d->iconFaceThumbThread->setThumbnailSize(d->faceSize);
                d->iconFaceThumbThread->setSendSurrogatePixmap(false);

                connect(d->iconFaceThumbThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                        this, SLOT(slotGotThumbnailFromIcon(LoadingDescription,QPixmap)),
                        Qt::QueuedConnection);
            }

            if (static_cast<TAlbum*>(album)->hasProperty(TagPropertyName::person()))
            {
                d->iconFaceThumbThread->find(ItemInfo::thumbnailIdentifier(id), faceRect);
            }
            else
            {
                d->iconTagThumbThread->find(ItemInfo::thumbnailIdentifier(id));
            }
        }
        else
        {
            if (!d->iconAlbumThumbThread)
            {
                d->iconAlbumThumbThread = new ThumbnailLoadThread();
                d->iconAlbumThumbThread->setThumbnailSize(d->iconSize);
                d->iconAlbumThumbThread->setSendSurrogatePixmap(false);

                connect(d->iconAlbumThumbThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                        this, SLOT(slotGotThumbnailFromIcon(LoadingDescription,QPixmap)),
                        Qt::QueuedConnection);
            }

            d->iconAlbumThumbThread->find(ItemInfo::thumbnailIdentifier(id));
        }

        // insert new entry to map, add album globalID

        QList<int>& list = d->idAlbumMap[QPair<qlonglong, QString>(id, faceRectStr)];
        list.removeAll(album->globalID());
        list.append(album->globalID());
    }
    else
    {
        // only add album global ID to list which is already inserted in map

        (*it).removeAll(album->globalID());
        (*it).append(album->globalID());
    }
}

void AlbumThumbnailLoader::setThumbnailSize(int size, int face)
{
    if ((d->iconSize == size) && (d->faceSize == face))
    {
        return;
    }

    d->iconSize = size;
    d->faceSize = face;

    // clear task list

    d->idAlbumMap.clear();

    // clear cached thumbnails

    d->thumbnailMap.clear();

    if (d->iconAlbumThumbThread)
    {
        d->iconAlbumThumbThread->stopLoading();
        d->iconAlbumThumbThread->setThumbnailSize(size);
    }

    if (d->iconFaceThumbThread)
    {
        d->iconFaceThumbThread->stopLoading();
        d->iconFaceThumbThread->setThumbnailSize(face);
    }

    if (d->iconTagThumbThread)
    {
        d->iconTagThumbThread->stopLoading();
        d->iconTagThumbThread->setThumbnailSize(size);
    }

    Q_EMIT signalReloadThumbnails();
}

int AlbumThumbnailLoader::thumbnailSize() const
{
    return d->iconSize;
}

void AlbumThumbnailLoader::slotGotThumbnailFromIcon(const LoadingDescription& loadingDescription, const QPixmap& thumbnail)
{
    // We need to find all albums for which the given url has been requested,
    // and Q_EMIT a signal for each album.

    QRect faceRect = QRect();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (loadingDescription.previewParameters.extraParameter.typeId() == QMetaType::QRect)

#else

    if (loadingDescription.previewParameters.extraParameter.type() == QVariant::Rect)

#endif

    {
        faceRect = loadingDescription.previewParameters.extraParameter.toRect();
    }

    // Simple way to put QRect into QMap

    QString faceRectStr     = QString(QLatin1String("%1%2%3%4"))
            .arg(faceRect.x()).arg(faceRect.y()).arg(faceRect.right()).arg(faceRect.bottom());

    ThumbnailIdentifier id  = loadingDescription.thumbnailIdentifier();
    IdAlbumMap::iterator it = d->idAlbumMap.find(QPair<qlonglong, QString>(id.id, faceRectStr));

    if (it != d->idAlbumMap.end())
    {
        AlbumManager* const manager = AlbumManager::instance();

        if (thumbnail.isNull())
        {
            // Loading failed

            for (QList<int>::const_iterator vit = (*it).constBegin() ; vit != (*it).constEnd() ; ++vit)
            {
                Album* const album = manager->findAlbum(*vit);

                if (album)
                {
                    Q_EMIT signalFailed(album);
                }
            }
        }
        else
        {
            // Loading succeeded

            for (QList<int>::const_iterator vit = (*it).constBegin() ; vit != (*it).constEnd() ; ++vit)
            {
                // look up with global id

                Album* const album = manager->findAlbum(*vit);

                if (album)
                {
                    d->thumbnailMap.insert(album->globalID(), thumbnail);

                    Q_EMIT signalThumbnail(album, thumbnail);
                }
            }
        }

        d->idAlbumMap.erase(it);
    }
}

void AlbumThumbnailLoader::slotDispatchThumbnailInternal(int albumID, const QPixmap& thumbnail)
{
    // for cached thumbnails

    AlbumManager* const manager = AlbumManager::instance();
    Album* const album          = manager->findAlbum(albumID);

    if (album)
    {
        if (thumbnail.isNull())
        {
            Q_EMIT signalFailed(album);
        }
        else
        {
            Q_EMIT signalThumbnail(album, thumbnail);
        }
    }
}

void AlbumThumbnailLoader::slotIconChanged(Album* album)
{
    if (!album || ((album->type() != Album::TAG) && (album->type() != Album::PHYSICAL)))
    {
        return;
    }

    d->thumbnailMap.remove(album->globalID());
}

/*
 * This code is maximally inefficient
QImage AlbumThumbnailLoader::getAlbumPreviewDirectly(PAlbum* const album, int size)
{
    if (album->iconId())
    {
        ThumbnailLoadThread* const thread    = new ThumbnailLoadThread;
        thread->setPixmapRequested(false);
        thread->setThumbnailSize(size);
        ThumbnailImageCatcher* const catcher = new ThumbnailImageCatcher(thread);
        catcher->setActive(true);
        catcher->thread()->find(ThumbnailIdentifier(album->iconId());
        catcher->enqueue();
        QList<QImage> images                 = catcher->waitForThumbnails();
        catcher->setActive(false);
        delete thread;
        delete catcher;

        if (!images.isEmpty())
        {
            return images[0];
        }
    }

    return loadIcon("folder", size).toImage();
}
*/

} // namespace Digikam

#include "moc_albumthumbnailloader.cpp"
