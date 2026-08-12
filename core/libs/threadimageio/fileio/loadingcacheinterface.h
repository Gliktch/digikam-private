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

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadingCacheInterface
{
public:

    static void initialize();

    /**
     * clean up cache at shutdown
     */
    static void cleanUp();

    /**
     * Remove an image from the cache
     * because it may have changed on disk
     */
    static void fileChanged(const QString& filePath, bool notify = true);

    /**
     * Connect the given object/slot to the signal
     *  void fileChanged(const QString& filePath);
     * which is emitted when the cache gains knowledge about a possible
     * change of this file on disk.
     */
    static void connectToSignalFileChanged(QObject* const object, const char* slot);

    /**
     * remove all images from the cache
     * (e.g. when loading settings changed)
     * Does not affect thumbnails.
     */
    static void cleanCache();

    /**
     * Remove all thumbnails from the thumbnail cache.
     * Does not affect main image cache.
     */
    static void cleanThumbnailCache();

    /**
     * add a copy of the image to cache
     */
    static void putImage(const QString& filePath, const DImg& img);

    /**
     * Set cache size in Megabytes.
     * Set to 0 to disable caching.
     */
    static void setCacheOptions(int cacheSize);

private:

    explicit LoadingCacheInterface();
    ~LoadingCacheInterface();

    /// @note disabled
    LoadingCacheInterface(const LoadingCacheInterface&)            = delete;
    LoadingCacheInterface& operator=(const LoadingCacheInterface&) = delete;
};

} // namespace Digikam
