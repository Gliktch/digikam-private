/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QString>
#include <QDebug>
#include <QMetaType>

namespace DigikamGenericRajcePlugin
{

struct RajceAlbum
{
    RajceAlbum()
    {
        isHidden   = false;
        isSecure   = false;
        photoCount = 0;
        id         = 0;
    }

    bool      isHidden;
    bool      isSecure;

    unsigned  photoCount;
    unsigned  id;

    QString   name;
    QString   description;
    QString   url;
    QString   thumbUrl;
    QString   bestQualityThumbUrl;

    QDateTime createDate;
    QDateTime updateDate;
    QDateTime validFrom;
    QDateTime validTo;
};

} // namespace DigikamGenericRajcePlugin

QTextStream& operator<<(QTextStream& str, const DigikamGenericRajcePlugin::RajceAlbum& a);
QDebug operator<<(QDebug d, const DigikamGenericRajcePlugin::RajceAlbum& a);

Q_DECLARE_METATYPE(DigikamGenericRajcePlugin::RajceAlbum)
