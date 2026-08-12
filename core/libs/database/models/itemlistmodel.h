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

#pragma once

// Local includes

#include "itemthumbnailmodel.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageChangeset;
class CollectionImageChangeset;

class DIGIKAM_DATABASE_EXPORT ItemListModel : public ItemThumbnailModel
{
    Q_OBJECT

public:

    explicit ItemListModel(QWidget* const parent);
    ~ItemListModel() override = default;

    /**
     * @note: necessary methods to add and remove ItemInfos to the model are inherited from ItemModel.
     * ItemModel::imageInfosRemoved() is emitted when images are removed from the model because they are removed in the database.
     */

protected Q_SLOTS:

    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
};

} // namespace Digikam
