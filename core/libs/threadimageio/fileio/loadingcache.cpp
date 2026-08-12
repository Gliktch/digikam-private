/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-11
 * Description : shared image loading and caching
 *
 * SPDX-FileCopyrightText: 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadingcache.h"

// Qt includes

#include <QCache>
#include <QHash>

// KDE includes

#include <kmemoryinfo.h>

// Local includes

#include "digikam_debug.h"
#include "iccsettings.h"
#include "metaengine.h"
#include "thumbnailsize.h"

namespace Digikam
{

class Q_DECL_HIDDEN LoadingCache::Private
{
public:

    explicit Private(LoadingCache* const qq)
        : q(qq)
    {
    }

    void mapImageFilePath(const QString& filePath, const QString& cacheKey);
    void mapThumbnailFilePath(const QString& filePath, const QString& cacheKey);
    void cleanUpImageFilePathHash();
    void cleanUpThumbnailFilePathHash();
    LoadingCacheFileWatch* fileWatch() const;

public:

    QCache<QString, DImg>           imageCache;
    QCache<QString, QImage>         thumbnailImageCache;
    QCache<QString, QPixmap>        thumbnailPixmapCache;
    QCache<QString, QPixmap>        bufferedTPixmapCache;
    QMultiHash<QString, QString>    imageFilePathHash;
    QMultiHash<QString, QString>    thumbnailFilePathHash;
    QHash<LoadingProcess*, QString> loadingDict;

    /// Note: Don't make the mutex recursive, we need to use a wait condition on it
    QMutex                          mutex;

    QWaitCondition                  condVar;

    LoadingCacheFileWatch*          watch = nullptr;
    LoadingCache*                   q     = nullptr;
};

LoadingCacheFileWatch* LoadingCache::Private::fileWatch() const
{
    // install default watch if no watch is set yet

    if (!watch)
    {
        q->setFileWatch(new LoadingCacheFileWatch);
    }

    return watch;
}

void LoadingCache::Private::mapImageFilePath(const QString& filePath, const QString& cacheKey)
{
    if (imageFilePathHash.size() > (5 * imageCache.size()))
    {
        cleanUpImageFilePathHash();
    }

    imageFilePathHash.insert(filePath, cacheKey);
}

void LoadingCache::Private::mapThumbnailFilePath(const QString& filePath, const QString& cacheKey)
{
    if (thumbnailFilePathHash.size() > (5 * (thumbnailImageCache.size() + thumbnailPixmapCache.size())))
    {
        cleanUpThumbnailFilePathHash();
    }

    thumbnailFilePathHash.insert(filePath, cacheKey);
}

void LoadingCache::Private::cleanUpImageFilePathHash()
{
    // Remove all entries from hash whose value is no longer a key in the cache

    QList<QString> keys = imageCache.keys();
    QMultiHash<QString, QString>::iterator it;

    for (it = imageFilePathHash.begin() ; it != imageFilePathHash.end() ; )
    {
        if (!keys.contains(it.value()))
        {
            fileWatch()->removeImage(it.key());
            it = imageFilePathHash.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LoadingCache::Private::cleanUpThumbnailFilePathHash()
{
    QList<QString> keys;

    keys += thumbnailImageCache.keys();
    keys += thumbnailPixmapCache.keys();

    QMultiHash<QString, QString>::iterator it;

    for (it = thumbnailFilePathHash.begin() ; it != thumbnailFilePathHash.end() ; )
    {
        if (!keys.contains(it.value()))
        {
            it = thumbnailFilePathHash.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

LoadingCache* LoadingCache::m_instance = nullptr;

LoadingCache* LoadingCache::cache()
{
    if (!m_instance)
    {
        m_instance = new LoadingCache;
    }

    return m_instance;
}

void LoadingCache::cleanUp()
{
    delete m_instance;
}

LoadingCache::LoadingCache()
    : d(new Private(this))
{
    KMemoryInfo memInfo;
    setCacheSize(qBound(100, (int)(memInfo.totalPhysical() / 1024.0 / 1024.0 * 0.06), 1024));

    // the pixmap number should not be based on system memory, it's graphics memory

    setThumbnailCacheSize(10, 200);

    // good place to call it here as LoadingCache is a singleton

    qRegisterMetaType<LoadingDescription>("LoadingDescription");
    qRegisterMetaType<DImg>("DImg");
    qRegisterMetaType<MetaEngineData>("MetaEngineData");

    connect(IccSettings::instance(), SIGNAL(signalICCSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)),
            this, SLOT(iccSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)));
}

LoadingCache::~LoadingCache()
{
    delete d->watch;
    delete d;
    m_instance = nullptr;
}

DImg* LoadingCache::retrieveImage(const QString& cacheKey) const
{
    QString filePath(d->imageFilePathHash.key(cacheKey));
    d->fileWatch()->checkFileWatch(filePath);

    return d->imageCache[cacheKey];
}

bool LoadingCache::putImage(const QString& cacheKey, const DImg& img, const QString& filePath) const
{
    bool isInserted = false;

    if (isCacheable(img))
    {
        int cost   = img.memorySize() / 1024;
        isInserted = d->imageCache.insert(cacheKey, new DImg(img), cost);

        if (isInserted && !filePath.isEmpty())
        {
            d->mapImageFilePath(filePath, cacheKey);
            d->fileWatch()->addedImage(filePath);
        }
    }

    return isInserted;
}

void LoadingCache::removeImage(const QString& cacheKey)
{
    d->imageCache.remove(cacheKey);
}

void LoadingCache::removeImages()
{
    d->imageCache.clear();
}

bool LoadingCache::isCacheable(const DImg& img) const
{
    // return whether image fits in cache

    return ((quint64)d->imageCache.maxCost() >= (img.memorySize() / 1024));
}

void LoadingCache::addLoadingProcess(LoadingProcess* const process)
{
    d->loadingDict.insert(process, process->cacheKey());
}

LoadingProcess* LoadingCache::retrieveLoadingProcess(const QString& cacheKey) const
{
    return d->loadingDict.key(cacheKey, nullptr);
}

void LoadingCache::removeLoadingProcess(LoadingProcess* const process)
{
    d->loadingDict.remove(process);
}

void LoadingCache::notifyNewLoadingProcess(LoadingProcess* const process, const LoadingDescription& description)
{
    for (QHash<LoadingProcess*, QString>::const_iterator it = d->loadingDict.constBegin() ;
         it != d->loadingDict.constEnd() ; ++it)
    {
        it.key()->notifyNewLoadingProcess(process, description);
    }
}

void LoadingCache::setCacheSize(int megabytes)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Allowing a cache size of" << megabytes << "MB";
    d->imageCache.setMaxCost(megabytes * 1024);
}

quint64 LoadingCache::getCacheSize() const
{
    return ((quint64)(d->imageCache.maxCost()) * 1024);
}

// --- Thumbnails ----

const QImage* LoadingCache::retrieveThumbnail(const QString& cacheKey) const
{
    return d->thumbnailImageCache[cacheKey];
}

const QPixmap* LoadingCache::retrieveThumbnailPixmap(const QString& cacheKey) const
{
    return d->thumbnailPixmapCache[cacheKey];
}

const QPixmap* LoadingCache::retrieveBufferedTPixmap(const QString& cacheKey) const
{
    return d->bufferedTPixmapCache[cacheKey];
}

bool LoadingCache::hasThumbnailPixmap(const QString& cacheKey) const
{
    return d->thumbnailPixmapCache.contains(cacheKey);
}

void LoadingCache::putThumbnail(const QString& cacheKey, const QImage& thumb, const QString& filePath)
{
    int cost = thumb.sizeInBytes();

    if (d->thumbnailImageCache.insert(cacheKey, new QImage(thumb), cost))
    {
        d->mapThumbnailFilePath(filePath, cacheKey);
    }
}

void LoadingCache::putThumbnail(const QString& cacheKey, const QPixmap& thumb, const QString& filePath)
{
    int cost = thumb.width() * thumb.height() * thumb.depth() / 8;

    if (d->thumbnailPixmapCache.insert(cacheKey, new QPixmap(thumb), cost))
    {
        d->mapThumbnailFilePath(filePath, cacheKey);

        d->bufferedTPixmapCache.remove(cacheKey);
    }
}

void LoadingCache::removeThumbnail(const QString& cacheKey)
{
    d->thumbnailImageCache.remove(cacheKey);
    d->thumbnailPixmapCache.remove(cacheKey);
}

void LoadingCache::removeThumbnails()
{
    d->thumbnailImageCache.clear();
    d->thumbnailPixmapCache.clear();
}

void LoadingCache::setThumbnailCacheSize(int numberOfQImages, int numberOfQPixmaps)
{
    d->thumbnailImageCache.setMaxCost(numberOfQImages                *
                                      ThumbnailSize::maxThumbsSize() *
                                      ThumbnailSize::maxThumbsSize() * 4);

    d->thumbnailPixmapCache.setMaxCost(numberOfQPixmaps               *
                                       ThumbnailSize::maxThumbsSize() *
                                       ThumbnailSize::maxThumbsSize() * QPixmap::defaultDepth() / 8);

    d->bufferedTPixmapCache.setMaxCost((numberOfQPixmaps / 2)         *
                                       ThumbnailSize::maxThumbsSize() *
                                       ThumbnailSize::maxThumbsSize() * QPixmap::defaultDepth() / 8);
}

void LoadingCache::setFileWatch(LoadingCacheFileWatch* const watch)
{
    delete d->watch;
    d->watch          = watch;
    d->watch->m_cache = this;
}

void LoadingCache::notifyFileChanged(const QString& filePath, bool notify)
{
    QList<QString> keys = d->imageFilePathHash.values(filePath);

    for (const QString& cacheKey : std::as_const(keys))
    {
        d->imageCache.remove(cacheKey);
    }

    keys = d->thumbnailFilePathHash.values(filePath);

    for (const QString& cacheKey : std::as_const(keys))
    {
        if (d->thumbnailPixmapCache.contains(cacheKey))
        {
            QPixmap* const thumb = d->thumbnailPixmapCache[cacheKey];

            if (thumb)
            {
                int cost = thumb->width() * thumb->height() * thumb->depth() / 8;

                d->bufferedTPixmapCache.insert(cacheKey, new QPixmap(*thumb), cost);
            }
        }

        d->thumbnailImageCache.remove(cacheKey);
        d->thumbnailPixmapCache.remove(cacheKey);
    }

    if (notify)
    {
        Q_EMIT fileChanged(filePath);
    }
}

void LoadingCache::iccSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous)
{
    if (
        (current.enableCM           != previous.enableCM)           ||
        (current.useManagedPreviews != previous.useManagedPreviews) ||
        (current.monitorProfile     != previous.monitorProfile)
       )
    {
        LoadingCache::CacheLock lock(this);
        removeImages();
        removeThumbnails();
    }
}

//---------------------------------------------------------------------------------------------------

LoadingCacheFileWatch::~LoadingCacheFileWatch()
{
    if (m_cache)
    {
        LoadingCache::CacheLock lock(m_cache);

        if (m_cache->d->watch == this)
        {
            m_cache->d->watch = nullptr;
        }
    }
}

void LoadingCacheFileWatch::addedImage(const QString& filePath)
{
    if (!m_cache || filePath.isEmpty())
    {
        return;
    }

    QFileInfo info(filePath);

    m_watchHash.insert(filePath, qMakePair(info.size(),
                                           info.lastModified()));
}

void LoadingCacheFileWatch::removeImage(const QString& filePath)
{
    m_watchHash.remove(filePath);
}

void LoadingCacheFileWatch::checkFileWatch(const QString& filePath)
{
    if (!m_cache || filePath.isEmpty())
    {
        return;
    }

    if (m_watchHash.contains(filePath))
    {
        QFileInfo info(filePath);
        QPair<qint64, QDateTime> pair = m_watchHash.value(filePath);

        if (
            (info.size()         != pair.first) ||
            (info.lastModified() != pair.second)
           )
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "LoadingCache file dirty:" << filePath;

            m_cache->notifyFileChanged(filePath);
            m_watchHash.remove(filePath);
        }
    }
}

void LoadingCacheFileWatch::notifyFileChanged(const QString& filePath)
{
    if (m_cache)
    {
        LoadingCache::CacheLock lock(m_cache);
        m_cache->notifyFileChanged(filePath);
    }
}

//---------------------------------------------------------------------------------------------------

LoadingCache::CacheLock::CacheLock(LoadingCache* const cache)
    : m_cache(cache)
{
    m_cache->d->mutex.lock();
}

LoadingCache::CacheLock::~CacheLock()
{
    m_cache->d->mutex.unlock();
}

void LoadingCache::CacheLock::wakeAll()
{
    // obviously the mutex is locked when this function is called

    m_cache->d->condVar.wakeAll();
}

void LoadingCache::CacheLock::timedWait()
{
    // same as above, the mutex is certainly locked

    m_cache->d->condVar.wait(&m_cache->d->mutex, 1000);
}

} // namespace Digikam

#include "moc_loadingcache.cpp"
