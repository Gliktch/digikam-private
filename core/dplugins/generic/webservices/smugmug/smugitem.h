/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

namespace DigikamGenericSmugPlugin
{

class SmugUser
{

public:

    SmugUser() = default;

    void clear()
    {
        email.clear();
        nickName.clear();
        displayName.clear();
        accountType.clear();
        userUri.clear();
        nodeUri.clear();
        folderUri.clear();
        fileSizeLimit = 0;
    }

public:

    QString email;
    QString nickName;
    QString displayName;
    QString accountType;
    QString userUri;
    QString nodeUri;
    QString folderUri;
    int     fileSizeLimit   = 0;
};

// ---------------------------------------------------------------------------------

class SmugAlbum
{

public:

    SmugAlbum() = default;

public:

    qint64  id              = -1;
    QString nodeID;

    QString name;
    QString key;
    QString title;
    QString description;
    QString keywords;

    qint64  categoryID      = -1;
    QString category;

    qint64  subCategoryID   = -1;
    QString subCategory;

    bool    isPublic        = true;
    QString password;
    bool    canShare        = true;
    QString passwordHint;

    int     imageCount      = 0;

    /// below fields only used by createAlbum (and not by listAlbums)
    qint64  tmplID          = -1;
    QString tmpl;

    // cppcheck-suppress constParameterCallback
    static bool lessThan(const SmugAlbum& a, const SmugAlbum& b)
    {
        return a.title.toLower() < b.title.toLower();
    }
};

// ---------------------------------------------------------------------------------

class SmugPhoto
{

public:

    SmugPhoto() = default;

public:

    qint64  id          = -1;

    QString key;
    QString caption;
    QString keywords;

    QString thumbURL;
    QString originalURL;
};

// ---------------------------------------------------------------------------------

class SmugAlbumTmpl
{

public:

    SmugAlbumTmpl() = default;

public:

    qint64  id          = -1;
    QString name;
    QString uri;

    bool    isPublic    = true;
    QString password;
    QString passwordHint;
};

// ---------------------------------------------------------------------------------

class SmugCategory
{

public:

    SmugCategory() = default;

public:

    qint64  id  = -1;
    QString name;
};

} // namespace DigikamGenericSmugPlugin
