/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ItemInfo common data
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QSize>
#include <QDateTime>
#include <QSharedData>
#include <QReadWriteLock>
#include <QWriteLocker>
#include <QReadLocker>

// Local includes

#include "coredburl.h"
#include "coredbalbuminfo.h"
#include "iteminfocache.h"
#include "digikam_globals.h"

namespace Digikam
{

class ItemInfoStatic
{
public:

    static void create();
    static void destroy();

    static ItemInfoCache* cache();

public:

    ItemInfoCache          m_cache;
    QReadWriteLock         m_lock;

    static ItemInfoStatic* m_instance;
};

// -----------------------------------------------------------------------------------

class ItemInfoReadLocker : public QReadLocker
{
public:

    explicit ItemInfoReadLocker()
        : QReadLocker(&ItemInfoStatic::m_instance->m_lock)
    {
    }
};

// -----------------------------------------------------------------------------------

class ItemInfoWriteLocker : public QWriteLocker
{
public:

    explicit ItemInfoWriteLocker()
        : QWriteLocker(&ItemInfoStatic::m_instance->m_lock)
    {
    }
};

// -----------------------------------------------------------------------------------

class ItemInfoData : public QSharedData
{
public:

    ItemInfoData()  = default;
    ~ItemInfoData() = default;

public:

    qlonglong                                id                         = -1;
    qlonglong                                currentReferenceImage      = -1;
    int                                      albumId                    = -1;
    int                                      albumRootId                = -1;
    QString                                  name;

    QString                                  defaultComment;
    QString                                  defaultTitle;
    quint8                                   pickLabel                  = NoPickLabel;
    quint8                                   colorLabel                 = NoColorLabel;
    qint8                                    rating                     = -1;
    DatabaseItem::Category                   category                   = DatabaseItem::UndefinedCategory;
    QString                                  format;
    QDateTime                                creationDate;
    QDateTime                                modificationDate;
    int                                      orientation                = 0;
    qlonglong                                fileSize                   = 0;
    qlonglong                                manualOrder                = 0;
    QString                                  uniqueHash;
    QSize                                    imageSize;
    QList<int>                               tagIds;
    int                                      faceCount                  = 0;
    int                                      unconfirmedFaceCount       = 0;
    QMap<QString, QString>                   faceSuggestions;

    double                                   longitude                  = 0;
    double                                   latitude                   = 0;
    double                                   altitude                   = 0;
    double                                   currentSimilarity          = 0.0;

    //! group leader, if the image is grouped
    qlonglong                                groupImage                 = -1;

    bool                                     hasCoordinates             = false;
    bool                                     hasAltitude                = false;

    bool                                     defaultTitleCached         = false;
    bool                                     defaultCommentCached       = false;
    bool                                     pickLabelCached            = false;
    bool                                     colorLabelCached           = false;
    bool                                     ratingCached               = false;
    bool                                     categoryCached             = false;
    bool                                     formatCached               = false;
    bool                                     creationDateCached         = false;
    bool                                     modificationDateCached     = false;
    bool                                     orientationCached          = false;
    bool                                     fileSizeCached             = false;
    bool                                     manualOrderCached          = false;
    bool                                     uniqueHashCached           = false;
    bool                                     imageSizeCached            = false;
    bool                                     tagIdsCached               = false;
    bool                                     positionsCached            = false;
    bool                                     groupImageCached           = false;
    bool                                     unconfirmedFaceCountCached = false;
    bool                                     faceSuggestionsCached      = false;
    bool                                     faceCountCached            = false;

    bool                                     invalid                    = false;
    bool                                     statusChanged              = false;

    // These two are initially true because we assume the data is there.
    // Once we query the data and find out it is missing, we set them to false.

    bool                                     hasVideoMetadata           = true;
    bool                                     hasImageMetadata           = true;


    DatabaseFields::VideoMetadataMinSizeType videoMetadataCached        = DatabaseFields::VideoMetadataNone;
    DatabaseFields::ImageMetadataMinSizeType imageMetadataCached        = DatabaseFields::ImageMetadataNone;

    typedef DatabaseFields::Hash<QVariant>   DatabaseFieldsHashRaw;
    DatabaseFieldsHashRaw                    databaseFieldsHashRaw;
};

} // namespace Digikam
