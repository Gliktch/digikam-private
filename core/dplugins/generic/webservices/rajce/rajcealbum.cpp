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

#include "rajcealbum.h"

QTextStream& operator << (QTextStream& str, const DigikamGenericRajcePlugin::RajceAlbum& a)
{
    str << "RajceAlbum[";
    str << "id=" << a.id;
    str << ", name='" << a.name << "'";
    str << ", description='" << a.description << "'";
    str << ", url='" << a.url << "'";
    str << ", createDate=" << a.createDate.toString(Qt::ISODate);
    str << ", updateDate=" << a.updateDate.toString(Qt::ISODate);
    str << ", validFrom=" << a.validFrom.toString(Qt::ISODate);
    str << ", validTo=" << a.validTo.toString(Qt::ISODate);
    str << ", isHidden=" << a.isHidden;
    str << ", isSecure=" << a.isSecure;
    str << ", photoCount=" << a.photoCount;
    str << ", thumbUrl='" << a.thumbUrl << "'";
    str << ", bestQualityThumbUrl='" << a.bestQualityThumbUrl << "'";
    str << "]";

    return str;
}

QDebug operator << (QDebug d, const DigikamGenericRajcePlugin::RajceAlbum& a)
{
    QString     s;
    QTextStream str(&s);

    str << a;

    d << *str.string();

    return d;
}
