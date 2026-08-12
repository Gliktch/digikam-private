/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - Tag methods.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"

namespace Digikam
{

void ItemIconView::setRecurseTags(bool recursive)
{
    d->iconView->itemAlbumModel()->setRecurseTags(recursive);
}

void ItemIconView::toggleTag(int tagID)
{
    ItemInfoList tagToRemove, tagToAssign;
    const ItemInfoList& selectedList = selectedInfoList(MetadataOps);

    for (const ItemInfo& info : selectedList)
    {
        if (info.tagIds().contains(tagID))
        {
            tagToRemove.append(info);
        }
        else
        {
            tagToAssign.append(info);
        }
    }

    if (!tagToAssign.isEmpty() && !tagToRemove.isEmpty())
    {
        tagToRemove.clear();
    }

    FileActionMngr::instance()->assignTag(tagToAssign, tagID);
    FileActionMngr::instance()->removeTag(tagToRemove, tagID);
}

void ItemIconView::slotAssignPickLabel(int pickId)
{
    const ItemInfoList& itemInfos = selectedInfoList(MetadataOps);
    int applyPickId               = NoPickLabel;

    for (const ItemInfo& info : itemInfos)
    {
        if (info.pickLabel() != pickId)
        {
            applyPickId = pickId;
            break;
        }
    }

    FileActionMngr::instance()->assignPickLabel(itemInfos, applyPickId);
}

void ItemIconView::slotAssignColorLabel(int colorId)
{
    const ItemInfoList& itemInfos = selectedInfoList(MetadataOps);
    int applyColorId              = NoColorLabel;

    for (const ItemInfo& info : itemInfos)
    {
        if (info.colorLabel() != colorId)
        {
            applyColorId = colorId;
            break;
        }
    }

    FileActionMngr::instance()->assignColorLabel(itemInfos, applyColorId);
}

void ItemIconView::slotAssignRating(int rating, bool toggle)
{
    const ItemInfoList& itemInfos = selectedInfoList(MetadataOps);
    int applyRating               = NoRating;

    if (toggle)
    {
        for (const ItemInfo& info : itemInfos)
        {
            if (info.rating() != rating)
            {
                applyRating = rating;
                break;
            }
        }
    }
    else
    {
        applyRating = rating;
    }

    FileActionMngr::instance()->assignRating(itemInfos, applyRating);
}

void ItemIconView::slotAssignTag()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToNewTagEdit();
}

void ItemIconView::slotAssignTag(int tagID)
{
    FileActionMngr::instance()->assignTags(selectedInfoList(MetadataOps), QList<int>() << tagID);
}

void ItemIconView::slotRemoveTag(int tagID)
{
    FileActionMngr::instance()->removeTags(selectedInfoList(MetadataOps), QList<int>() << tagID);

    /**
     * Implementation for Automatic Icon Removal of
     * Confirmed Tags.
     * QTimer to ensure TagRemoval is complete.
     */
    if (!FaceTags::isSystemPersonTagId(tagID))
    {
        QTimer::singleShot(200, this,
                           [tagID]()
            {
                int count = CoreDbAccess().db()->getNumberOfImagesInTagProperties(tagID,
                                                                                  ImageTagPropertyName::tagRegion());

                /**
                 * If the face just removed was the final face
                 * associated with that Tag, reset Tag Icon.
                 */
                if (count == 0)
                {
                    TAlbum* const album = AlbumManager::instance()->findTAlbum(tagID);

                    if (album && (album->iconId() != 0))
                    {
                        QString err;

                        if (!AlbumManager::instance()->updateTAlbumIcon(album, QString(),
                                                                        0, err))
                        {
                            qCDebug(DIGIKAM_GENERAL_LOG) << err;
                        }
                    }
                }
            }
        );
    }
}

void ItemIconView::slotNewTag()
{
    QList<TAlbum*> talbums = AlbumManager::instance()->currentTAlbums();

    if (!talbums.isEmpty())
    {
        d->tagModificationHelper->slotTagNew(talbums.first());
    }
}

void ItemIconView::slotDeleteTag()
{
    QList<TAlbum*> talbums = AlbumManager::instance()->currentTAlbums();

    if (!talbums.isEmpty())
    {
        d->tagModificationHelper->slotTagDelete(talbums.first());
    }
}

void ItemIconView::slotEditTag()
{
    QList<TAlbum*> talbums = AlbumManager::instance()->currentTAlbums();

    if (!talbums.isEmpty())
    {
        d->tagModificationHelper->slotTagEdit(talbums.first());
    }
}

void ItemIconView::slotOpenTagsManager()
{
    TagsManager* const tagMngr = TagsManager::instance();
    tagMngr->show();
    tagMngr->activateWindow();
    tagMngr->raise();
}

/*
void ItemIconView::slotRatingChanged(const QUrl& url, int rating)
{
    rating        = qMin(RatingMax, qMax(RatingMin, rating));
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignRating(info, rating);
    }
}

void ItemIconView::slotColorLabelChanged(const QUrl& url, int color)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignColorLabel(info, color);
    }
}

void ItemIconView::slotPickLabelChanged(const QUrl& url, int pick)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignPickLabel(info, pick);
    }
}

void ItemIconView::slotToggleTag(const QUrl& url, int tagID)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        if (info.tagIds().contains(tagID))
        {
            FileActionMngr::instance()->removeTag(info, tagID);
        }
        else
        {
            FileActionMngr::instance()->assignTag(info, tagID);
        }
    }
}
*/

} // namespace Digikam
