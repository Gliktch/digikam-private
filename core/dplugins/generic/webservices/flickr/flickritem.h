/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>
#include <QString>

// Local includes

#include "flickrlist.h"

namespace DigikamGenericFlickrPlugin
{

class FPhotoInfo
{

public:

    FPhotoInfo() = default;

    bool                    is_public       = false;
    bool                    is_friend       = false;
    bool                    is_family       = false;

    QString                 title;
    QString                 description;
    qlonglong               size            = 0;
    QStringList             tags;

    FlickrList::SafetyLevel safety_level    = FlickrList::SAFE;
    FlickrList::ContentType content_type    = FlickrList::PHOTO;
};

// -------------------------------------------------------------

class FPhotoSet
{

public:

    FPhotoSet() = default;

    QString id          = QLatin1String("-1");
    QString primary;    ///< "2483"
    QString secret;     ///< "abcdef"
    QString server;
    QString photos;
    QString title;
    QString description;
};

} // namespace DigikamGenericFlickrPlugin
