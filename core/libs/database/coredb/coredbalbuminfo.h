/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-21
 * Description : Structures to define Albums used in CoreDb
 *
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QList>
#include <QDateTime>

// Local includes

#include "coredbconstants.h"

namespace Digikam
{

typedef QPair<int, int> YearMonth;

/**
 * @class AlbumRootInfo
 */
class AlbumRootInfo
{
public:

    AlbumRootInfo() = default;

public:

    int     id                  = 0;
    QString label;
    int     status              = 0;
    int     type                = 0;
    QString identifier;
    QString specificPath;
    int     caseSensitivity     = 0;
};

// --------------------------------------------------------------------------

/**
 * @class AlbumInfo
 * A container class for transporting album information
 * from the database to AlbumManager
 */
class AlbumInfo
{
public:

    typedef QList<AlbumInfo> List;

public:

    AlbumInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

    /**
     * needed for sorting
     */
    bool operator<(const AlbumInfo& info) const
    {
        // include album root id?

        return (relativePath < info.relativePath);
    }

public:

    int       id            = 0;
    int       albumRootId   = 0;
    QString   relativePath;
    QString   caption;
    QString   category;
    QDate     date;
    qlonglong iconId        = 0;
};

// --------------------------------------------------------------------------

/**
 * @class TagInfo
 * A container class for transporting tag information
 * from the database to AlbumManager
 */
class TagInfo
{
public:

    typedef QList<TagInfo> List;

public:

    TagInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

    bool operator<(const TagInfo& info) const
    {
        return (name < info.name);
    }

public:

    int       id        = 0;
    int       pid       = 0;
    QString   name;
    QString   icon;
    qlonglong iconId    = 0;
};

// --------------------------------------------------------------------------

/**
 * @class SearchInfo
 * A container class for transporting search information
 * from the database to AlbumManager
 */
class SearchInfo
{
public:

    typedef QList<SearchInfo> List;

public:

    SearchInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

    /**
     * needed for sorting
     */
    bool operator<(const SearchInfo& info) const
    {
        return (id < info.id);
    }

public:

    int                  id     = 0;
    QString              name;
    DatabaseSearch::Type type   = DatabaseSearch::UndefinedType;
    QString              query;
};

// --------------------------------------------------------------------------

class AlbumShortInfo
{
public:

    AlbumShortInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

public:

    int     id          = 0;
    QString relativePath;
    int     albumRootId = 0;
};

// --------------------------------------------------------------------------

class TagShortInfo
{
public:

    TagShortInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

public:

    int     id      = 0;
    int     pid     = 0;
    QString name;
};

// --------------------------------------------------------------------------

class ItemShortInfo
{
public:

    ItemShortInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

public:

    qlonglong id            = 0;
    QString   itemName;
    int       albumID       = 0;
    int       albumRootID   = 0;
    QString   album;
};

// --------------------------------------------------------------------------

class ItemScanInfo
{
public:

    ItemScanInfo() = default;

    bool isNull() const
    {
        return (id == 0);
    }

public:

    qlonglong              id               = 0;
    int                    albumID          = 0;
    QString                itemName;
    DatabaseItem::Status   status           = DatabaseItem::UndefinedStatus;
    DatabaseItem::Category category         = DatabaseItem::UndefinedCategory;
    QDateTime              modificationDate;
    qlonglong              fileSize         = 0;
    QString                uniqueHash;
};

// --------------------------------------------------------------------------

class CommentInfo
{
public:

    CommentInfo() = default;

    bool isNull() const
    {
        return (id == -1);
    }

public:

    int                   id        = -1;
    qlonglong             imageId   = -1;
    DatabaseComment::Type type      = DatabaseComment::UndefinedType;
    QString               author;
    QString               language;
    QDateTime             date;
    QString               comment;
};

// --------------------------------------------------------------------------

class CopyrightInfo
{
public:

    CopyrightInfo() = default;

    bool isNull() const
    {
        return (id == -1);
    }

public:

    qlonglong id        = -1;
    QString   property;
    QString   value;
    QString   extraValue;
};

// --------------------------------------------------------------------------

class ImageHistoryEntry
{
public:

    ImageHistoryEntry() = default;

    bool isNull() const
    {
        return (imageId == 0);
    }

public:

    qlonglong imageId   = 0;
    QString   uuid;
    QString   history;
};

// --------------------------------------------------------------------------

class ImageRelation
{
public:

    ImageRelation() = default;

public:

    qlonglong              subjectId = 0;
    qlonglong              objectId  = 0;
    DatabaseRelation::Type type      = DatabaseRelation::UndefinedType;
};

// --------------------------------------------------------------------------

class TagProperty
{
public:

    TagProperty() = default;

    bool isNull() const
    {
        return (tagId == -1);
    }

public:

    int     tagId   = -1;
    QString property;
    QString value;
};

// --------------------------------------------------------------------------

class ImageTagProperty
{
public:

    ImageTagProperty() = default;

    bool isNull() const
    {
        return (imageId == -1);
    }

public:

    qlonglong imageId   = -1;
    int       tagId     = -1;
    QString   property;
    QString   value;
};

} // namespace Digikam
