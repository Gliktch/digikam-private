/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-11-15
 * Description : Information for thumbnails
 *
 * SPDX-FileCopyrightText: 2006-2014 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"

namespace Digikam
{

class DIGIKAM_EXPORT ThumbnailIdentifier
{
public:

    ThumbnailIdentifier() = default;

    explicit ThumbnailIdentifier(const QString& path)
        : filePath(path)
    {
    }

    /**
     * The file path from which the thumbnail shall be generated
     */
    QString   filePath;

    /**
     * The database id, which needs to be translated to uniqueHash + fileSize
     */
    qlonglong id = 0;
};

class DIGIKAM_EXPORT ThumbnailInfo : public ThumbnailIdentifier
{
public:

    ThumbnailInfo()  = default;
    ~ThumbnailInfo() = default;

    /**
     * If available, the uniqueHash + fileSize pair for identification
     * of the original file by content.
     */
    QString   uniqueHash;
    qlonglong fileSize      = 0;

    /**
     * If the original file is at all accessible on disk.
     * May be false if a file on a removable device is used.
     */
    bool      isAccessible  = false;

    /**
     * The modification date of the original file.
     * Thumbnail will be regenerated if thumb's modification date is older than this.
     */
    QDateTime modificationDate;

    /**
     * Gives a hint at the orientation of the image.
     * This can be used to supersede the Exif information in the file.
     * Will not be used if DMetadata::ORIENTATION_UNSPECIFIED (default value)
     */
    int       orientationHint = DMetadata::ORIENTATION_UNSPECIFIED;

    /**
     * The file name (the name, not the directory)
     */
    QString   fileName;

    /**
     * The mime type of the original file.
     * Currently "image" or "video" otherwise empty.
     */
    QString   mimeType;

    /**
     * A custom identifier, if neither filePath nor uniqueHash are applicable.
     */
    QString   customIdentifier;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbnailInfoProvider
{
public:

    ThumbnailInfoProvider()          = default;
    virtual ~ThumbnailInfoProvider() = default;

    virtual ThumbnailInfo thumbnailInfo(const ThumbnailIdentifier&) = 0;

private:

    Q_DISABLE_COPY(ThumbnailInfoProvider)
};

} // namespace Digikam
