/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-1
 * Description : Showfoto item info container
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotoiteminfo.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

namespace ShowFoto
{

bool ShowfotoItemInfo::isNull() const
{
    return (
            (size == -1)     &&
            (id   == -1)     &&
            name.isNull()    &&
            folder.isNull()  &&
            mime.isNull()
           );
}

bool ShowfotoItemInfo::operator==(const ShowfotoItemInfo& info) const
{
    bool b1  = (size      == info.size);
    bool b2  = (name      == info.name);
    bool b3  = (folder    == info.folder);
    bool b4  = (mime      == info.mime);
    bool b5  = (id        == info.id);
    bool b6  = (photoInfo == info.photoInfo);

    return (b1 && b2 && b3 && b4 && b5 && b6);
}

bool ShowfotoItemInfo::operator!=(const ShowfotoItemInfo& info) const
{
    return (!operator==(info));
}

QDataStream& operator<<(QDataStream& ds, const ShowfotoItemInfo& info)
{
    ds << info.name;
    ds << info.folder;
    ds << info.mime;
    ds << info.size;
    ds << info.id;
    ds << info.photoInfo;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, ShowfotoItemInfo& info)
{
    ds >> info.name;
    ds >> info.folder;
    ds >> info.mime;
    ds >> info.size;
    ds >> info.id;
    ds >> info.photoInfo;

    return ds;
}

QDebug operator<<(QDebug dbg, const ShowfotoItemInfo& info)
{
    dbg.nospace() << "ShowfotoItemInfo::size: "
                  << info.size << Qt::endl;
    dbg.nospace() << "ShowfotoItemInfo::name: "
                  << info.name << Qt::endl;
    dbg.nospace() << "ShowfotoitemInfo::folder: "
                  << info.folder << Qt::endl;
    dbg.nospace() << "ShowfotoItemInfo::mime: "
                  << info.mime << Qt::endl;
    dbg.nospace() << "ShowfotoItemInfo::id: "
                  << info.id << Qt::endl << Qt::endl;

    return dbg.space();
}

ShowfotoItemInfo ShowfotoItemInfo::itemInfoFromFile(const QFileInfo& inf)
{
    ShowfotoItemInfo iteminfo;
    QScopedPointer<DMetadata> meta(new DMetadata);

    // And open all items in image editor.

    iteminfo.name      = inf.fileName();
    iteminfo.mime      = inf.suffix();
    iteminfo.size      = inf.size();
    iteminfo.folder    = inf.path();
    iteminfo.url       = QUrl::fromLocalFile(inf.filePath());
    iteminfo.dtime     = inf.birthTime();

    meta->load(inf.filePath());
    iteminfo.ctime     = meta->getItemDateTime();
    iteminfo.width     = meta->getItemDimensions().width();
    iteminfo.height    = meta->getItemDimensions().height();
    iteminfo.photoInfo = meta->getPhotographInformation();

    return iteminfo;
}

} // namespace Showfoto
