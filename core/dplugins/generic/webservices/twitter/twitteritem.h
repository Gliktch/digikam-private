/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

 // Qt includes

#include <QString>

namespace DigikamGenericTwitterPlugin
{

class TwUser
{
public:

    TwUser() = default;

    void clear()
    {
        id         = 0;
        name.clear();
        uploadPerm = true;
    }

    long long id            = 0;

    QString   name;
    QString   profileURL;
    bool      uploadPerm    = false;
};

// ---------------------------------------------------------------

class TwAlbum
{
public:

    TwAlbum() = default;

    QString   id;

    QString   title;
    QString   description;
    QString   location;
    QString   url;
};

// ---------------------------------------------------------------

class TwPhoto
{
public:

    TwPhoto() = default;

    QString id;
    QString caption;
    QString thumbURL;
    QString originalURL;
};

} // namespace DigikamGenericTwitterPlugin
