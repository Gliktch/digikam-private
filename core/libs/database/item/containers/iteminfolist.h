/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Container for image info objects
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "iteminfo.h"
#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{

class ItemInfo;

// NOTE: implementations of batch loading methods:
// See imageinfo.cpp (next to the corresponding single-item implementation)

class DIGIKAM_DATABASE_EXPORT ItemInfoList : public QList<ItemInfo>
{
public:

    ItemInfoList() = default;
    explicit ItemInfoList(const QList<ItemInfo>& list);
    explicit ItemInfoList(const QList<qlonglong>& idList);

    QList<qlonglong> toImageIdList()  const;
    QList<QUrl>      toImageUrlList() const;

    void loadGroupImageIds()          const;
    void loadTagIds()                 const;

    bool static namefileLessThan(const ItemInfo& d1, const ItemInfo& d2);

    /**
     * @brief singleGroupMainItem
     * @return If the list contains of items of only one group including the
     * main item, this main item is returned, otherwise a null ItemInfo.
     */
    ItemInfo singleGroupMainItem()    const;
};

typedef ItemInfoList::iterator ItemInfoListIterator;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemInfoList)
