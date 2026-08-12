/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries, using AlbumManager
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "itemfiltermodel.h"

namespace Digikam
{

class Album;
class ItemAlbumModel;
class ItemAlbumFilterModelPrivate;
class FaceTagsIface;

class ItemAlbumFilterModel : public ItemFilterModel
{
    Q_OBJECT

public:

    explicit ItemAlbumFilterModel(QObject* const parent = nullptr);
    ~ItemAlbumFilterModel()                                           override;

    void            setSourceItemModel(ItemAlbumModel* model);
    ItemAlbumModel* sourceModel()                               const;

    void setItemFilterSettings(const ItemFilterSettings& settings)    override;

protected:

    int compareInfosCategories(const ItemInfo& left,
                               const ItemInfo& right)           const override;

    int compareInfosCategories(const ItemInfo& left,
                               const ItemInfo& right,
                               const FaceTagsIface& leftFace,
                               const FaceTagsIface& rightFace)  const override;

protected Q_SLOTS:

    void slotAlbumRenamed(Album* album);
    void slotAlbumAdded(Album* album);
    void slotAlbumAboutToBeDeleted(Album* album);
    void slotAlbumsCleared();

private:

    void albumChange(Album* album);

private Q_SLOTS:

    void slotDelayedAlbumNamesTimer();
    void slotDelayedTagNamesTimer();

private:

    Q_DECLARE_PRIVATE(ItemAlbumFilterModel)
};

} // namespace Digikam
