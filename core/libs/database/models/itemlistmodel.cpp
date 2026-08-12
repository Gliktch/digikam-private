/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-06
 * Description : An item model based on a static list
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemlistmodel.h"

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "iteminfo.h"
#include "iteminfolist.h"

namespace Digikam
{

ItemListModel::ItemListModel(QWidget* const parent)
    : ItemThumbnailModel(parent)
{
    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));
}

void ItemListModel::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (isEmpty())
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
        {
            break;
        }

        case CollectionImageChangeset::Deleted:
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
        {
            removeItemInfos(ItemInfoList(changeset.ids()));
            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace Digikam

#include "moc_itemlistmodel.cpp"
