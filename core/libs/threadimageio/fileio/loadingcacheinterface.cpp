/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-06
 * Description : shared image loading and caching
 *
 * SPDX-FileCopyrightText: 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadingcacheinterface.h"

// Local includes

#include "loadingcache.h"

namespace Digikam
{

void LoadingCacheInterface::initialize()
{
    LoadingCache::cache();
}

void LoadingCacheInterface::cleanUp()
{
    LoadingCache::cleanUp();
}

void LoadingCacheInterface::fileChanged(const QString& filePath, bool notify)
{
    LoadingCache* const cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);

    cache->notifyFileChanged(filePath, notify);
}

void LoadingCacheInterface::connectToSignalFileChanged(QObject* const object, const char* slot)
{
    const LoadingCache* const cache = LoadingCache::cache();

    QObject::connect(cache, SIGNAL(fileChanged(QString)),
                     object, slot,
                     Qt::QueuedConnection);

    // make it a queued connection because the signal is emitted when the CacheLock is held!
}

void LoadingCacheInterface::cleanCache()
{
    LoadingCache* const cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);

    cache->removeImages();
}

void LoadingCacheInterface::cleanThumbnailCache()
{
    LoadingCache* const cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);

    cache->removeThumbnails();
}

void LoadingCacheInterface::putImage(const QString& filePath, const DImg& img)
{
    LoadingCache* const cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);

    if (cache->isCacheable(img))
    {
        cache->putImage(filePath, img, filePath);
    }
}

void LoadingCacheInterface::setCacheOptions(int cacheSize)
{
    LoadingCache* const cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);

    cache->setCacheSize(cacheSize);
}

} // namespace Digikam
