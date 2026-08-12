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

#pragma once

// Qt includes

#include <QList>
#include <QDateTime>

namespace Digikam
{

class DTrashItemInfo
{

public:

    DTrashItemInfo() = default;

    bool isNull()                                   const;
    bool operator==(const DTrashItemInfo& itemInfo) const;

public:

    QString   trashPath;
    QString   jsonFilePath;
    QString   collectionPath;
    QString   collectionRelativePath;
    QDateTime deletionTimestamp;
    qlonglong imageId                = -1;
};

typedef QList<DTrashItemInfo> DTrashItemInfoList;

//! qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const DTrashItemInfo& info);

} // namespace Digikam
