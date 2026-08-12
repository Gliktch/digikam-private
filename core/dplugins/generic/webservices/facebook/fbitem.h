/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "wsitem.h"

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

class FbUser
{
public:

    FbUser() = default;

    void clear()
    {
        id.clear();
        name.clear();
        profileURL = QLatin1String("https://www.facebook.com");
        uploadPerm = true;
    }

    QString   id;

    QString   name;
    QString   profileURL;
    bool      uploadPerm = false;
};

// ---------------------------------------------------------------

enum FbPrivacy
{
    FB_ME       = 0,
    FB_FRIENDS  = 1,
    FB_FRIENDS_OF_FRIENDS,
//     FB_NETWORKS, //NETWORK is deprecated in latest version of Graph API
    FB_EVERYONE,
    FB_CUSTOM
};

// ---------------------------------------------------------------

class FbAlbum: public WSAlbum
{
public:

    explicit FbAlbum()
      : WSAlbum()
    {
    }

    explicit FbAlbum(const WSAlbum& baseAlbum)
      : WSAlbum(baseAlbum)
    {
    }

    FbPrivacy privacy = FB_FRIENDS;
};

// ---------------------------------------------------------------

class FbPhoto
{
public:

    FbPhoto() = default;

    QString id;

    QString caption;
    QString thumbURL;
    QString originalURL;
};

} // namespace DigikamGenericFaceBookPlugin
