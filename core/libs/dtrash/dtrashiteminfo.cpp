/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-08
 * Description : DTrash item info container
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dtrashiteminfo.h"

// Qt includes

#include <QDebug>

namespace Digikam
{

bool DTrashItemInfo::isNull() const
{
    return (
            trashPath.isEmpty()              &&
            jsonFilePath.isEmpty()           &&
            collectionPath.isEmpty()         &&
            collectionRelativePath.isEmpty() &&
            deletionTimestamp.isNull()       &&
            (imageId == -1)
           );
}

bool DTrashItemInfo::operator==(const DTrashItemInfo& itemInfo) const
{
    return (trashPath == itemInfo.trashPath);
}

QDebug operator<<(QDebug dbg, const DTrashItemInfo& info)
{
    dbg.nospace() << "DTrashItemInfo:";
    dbg.nospace() << "\ntrashPath: "         << info.trashPath;
    dbg.nospace() << "\njsonFilePath: "      << info.jsonFilePath;
    dbg.nospace() << "\nCollectionPath: "    << info.collectionPath;
    dbg.nospace() << "\nRelativePath: "      << info.collectionRelativePath;
    dbg.nospace() << "\nDeletionTimestamp: " << info.deletionTimestamp.toString();
    dbg.nospace() << "\nImage id: "          << QString::number(info.imageId) << "\n";

    return dbg.space();
}

} // namespace Digikam
