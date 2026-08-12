/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QString>

namespace DigikamGenericGoogleServicesPlugin
{

enum GoogleService
{
    GDrive = 1,
    GPhotoExport,
    GPhotoImport
};

// -----------------------------------------------------------

class GSPhoto
{
public:

    GSPhoto() = default;

public:

    QString     id              = QLatin1String("-1");
    QString     title;
    QString     description;

    QString     timestamp;
    QString     creationTime;

    QString     location;
    QString     gpsLon;
    QString     gpsLat;

    bool        canComment      = true;
    QStringList tags;

    QString     width;
    QString     height;

    QString     mimeType;
    QString     baseUrl;
    QUrl        originalURL;
    QUrl        thumbURL;
    QUrl        editUrl;
};

// -----------------------------------------------------------

class GSFolder
{
public:

    GSFolder() = default;

public:

    QString     id          = QLatin1String("-1");
    QString     title       = QLatin1String("<auto-create>");
    QString     timestamp;
    QString     description;
    QString     location;
    bool        canComment  = true;
    bool        isWriteable = true;
    QStringList tags;
    QString     url;
};

} // namespace DigikamGenericGoogleServicesPlugin
