/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class - Tags methods.
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "contextmenuhelper_p.h"

namespace Digikam
{

void ContextMenuHelper::addActionNewTag(TagModificationHelper* const helper, TAlbum* const tag)
{
    QAction* const newTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")),
                                              i18nc("@action: context menu", "New Tag..."), this);
    addAction(newTagAction);
    helper->bindTag(newTagAction, tag);

    connect(newTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagNew()));
}

void ContextMenuHelper::addActionDeleteTag(TagModificationHelper* const helper, TAlbum* const tag)
{
    QAction* const deleteTagAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                 i18nc("@action: context menu", "Delete Tag"), this);
    addAction(deleteTagAction);
    helper->bindTag(deleteTagAction, tag);

    connect(deleteTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagDelete()));
}

void ContextMenuHelper::addActionDeleteTags(TagModificationHelper* const helper, const QList<TAlbum*>& tags)
{
    QAction* const deleteTagsAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                  i18nc("@action: context menu", "Delete Tags"), this);
    addAction(deleteTagsAction);
    helper->bindMultipleTags(deleteTagsAction, tags);

    connect(deleteTagsAction, SIGNAL(triggered()),
            helper, SLOT(slotMultipleTagDel()));
}

void ContextMenuHelper::addActionTagToFaceTag(TagModificationHelper* const helper, TAlbum* const tag)
{
    QAction* const tagToFaceTagAction = new QAction(QIcon::fromTheme(QLatin1String("smiley")),
                                                    i18nc("@action: context menu", "Mark As Face Tag"), this);
    addAction(tagToFaceTagAction);
    helper->bindTag(tagToFaceTagAction, tag);

    connect(tagToFaceTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagToFaceTag()));
}

void ContextMenuHelper::addActionTagsToFaceTags(TagModificationHelper* const helper, const QList<TAlbum*>& tags)
{
    QAction* const tagToFaceTagsAction = new QAction(QIcon::fromTheme(QLatin1String("smiley")),
                                                     i18nc("@action: context menu", "Mark As Face Tags"), this);
    addAction(tagToFaceTagsAction);
    helper->bindMultipleTags(tagToFaceTagsAction, tags);

    connect(tagToFaceTagsAction, SIGNAL(triggered()),
            helper, SLOT(slotMultipleTagsToFaceTags()));
}

void ContextMenuHelper::addActionEditTag(TagModificationHelper* const helper, TAlbum* const tag)
{
    QAction* const editTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-properties")),
                                               i18nc("@action: edit tag properties", "Properties..."), this);

    // This is only for the user to give a hint for the shortcut key

    editTagAction->setShortcut(Qt::ALT | Qt::Key_Return);
    addAction(editTagAction);
    helper->bindTag(editTagAction, tag);

    connect(editTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagEdit()));
}

void ContextMenuHelper::addActionDeleteFaceTag(TagModificationHelper* const helper, TAlbum* const tag)
{
    QAction* const deleteFaceTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag")),
                                                     i18nc("@action: context menu", "Unmark Tag As Face"), this);
    deleteFaceTagAction->setWhatsThis(i18nc("@info: context menu", "Removes the face property from the selected tag "
                                            "and the face region from the contained images. "
                                            "Can also untag the images if wished."));
    addAction(deleteFaceTagAction);
    helper->bindTag(deleteFaceTagAction, tag);

    connect(deleteFaceTagAction, SIGNAL(triggered()),
            helper, SLOT(slotFaceTagDelete()));
}

void ContextMenuHelper::addActionDeleteFaceTags(TagModificationHelper* const helper, const QList<TAlbum*>& tags)
{
    QAction* const deleteFaceTagsAction = new QAction(QIcon::fromTheme(QLatin1String("tag")),
                                                      i18nc("@action: context menu", "Unmark Tags As Face"), this);
    deleteFaceTagsAction->setWhatsThis(i18nc("@info: context menu", "Removes the face property from the selected tags "
                                             "and the face region from the contained images. "
                                             "Can also untag the images if wished."));
    addAction(deleteFaceTagsAction);
    helper->bindMultipleTags(deleteFaceTagsAction, tags);

    connect(deleteFaceTagsAction, SIGNAL(triggered()),
            helper, SLOT(slotMultipleFaceTagDel()));
}

void ContextMenuHelper::addAssignTagsMenu(const imageIds& ids)
{
    setSelectedIds(ids);

    QMenu* const assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::RECENTLYASSIGNED, d->parent);
    assignTagsPopup->menuAction()->setText(i18nc("@action: context menu", "A&ssign Tag"));
    assignTagsPopup->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    d->parent->addMenu(assignTagsPopup);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalAssignTag(int)));

    connect(assignTagsPopup, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));
}

void ContextMenuHelper::addRemoveTagsMenu(const imageIds& ids)
{
    setSelectedIds(ids);

    QMenu* const removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->parent);
    removeTagsPopup->menuAction()->setText(i18nc("@action: context menu", "R&emove Tag"));
    removeTagsPopup->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    d->parent->addMenu(removeTagsPopup);

    // Performance: Only check for tags if there are <250 images selected
    // Otherwise enable it regardless if there are tags or not

    if (ids.count() < 250)
    {
        QList<int> tagIDs = CoreDbAccess().db()->getItemCommonTagIDs(ids);
        bool enable       = false;

        for (int tag : std::as_const(tagIDs))
        {
            if (
                (TagsCache::instance()->colorLabelForTag(tag) == -1)    &&
                (TagsCache::instance()->pickLabelForTag(tag)  == -1)    &&
                (TagsCache::instance()->isInternalTag(tag)    == false)
               )
            {
                enable = true;
                break;
            }
        }

        removeTagsPopup->menuAction()->setEnabled(enable);
    }

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalRemoveTag(int)));
}

void ContextMenuHelper::addRemoveAllTags(const imageIds& ids)
{
    setSelectedIds(ids);

    QAction* const removeAllTagsAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                     i18nc("@action: context menu", "Remove all Tags"), this);
    addAction(removeAllTagsAction);

    connect(removeAllTagsAction, SIGNAL(triggered()),
            this, SLOT(slotRemoveAllTags()));
}

void ContextMenuHelper::addLabelsAction()
{
    QMenu* const menuLabels           = new QMenu(i18nc("@action: context menu", "Assign Labe&ls"), d->parent);
    PickLabelMenuAction* const pmenu  = new PickLabelMenuAction(d->parent);
    ColorLabelMenuAction* const cmenu = new ColorLabelMenuAction(d->parent);
    RatingMenuAction* const rmenu     = new RatingMenuAction(d->parent);
    menuLabels->addAction(pmenu->menuAction());
    menuLabels->addAction(cmenu->menuAction());
    menuLabels->addAction(rmenu->menuAction());
    addSubMenu(menuLabels);

    connect(pmenu, SIGNAL(signalPickLabelChanged(int)),
            this, SIGNAL(signalAssignPickLabel(int)));

    connect(cmenu, SIGNAL(signalColorLabelChanged(int)),
            this, SIGNAL(signalAssignColorLabel(int)));

    connect(rmenu, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalAssignRating(int)));
}

void ContextMenuHelper::addCreateTagFromAddressbookMenu()
{

#ifdef HAVE_AKONADICONTACT

    AkonadiIface* const abc = new AkonadiIface(d->parent);

    connect(abc, SIGNAL(signalContactTriggered(QString)),
            this, SIGNAL(signalAddNewTagFromABCMenu(QString)));

    // AkonadiIface instance will be deleted with d->parent.

#endif

}

void ContextMenuHelper::addGotoMenu(const imageIds& ids)
{
    if (d->gotoAlbumAction && d->gotoDateAction)
    {
        return;
    }

    setSelectedIds(ids);

    // the currently selected image is always the first item

    ItemInfo item;

    if (!d->selectedIds.isEmpty())
    {
        item = ItemInfo(d->selectedIds.first());
    }

    if (item.isNull())
    {
        return;
    }

    // when more then one item is selected, don't add the menu

    if (d->selectedIds.count() > 1)
    {
        return;
    }

    d->gotoAlbumAction    = new QAction(QIcon::fromTheme(QLatin1String("folder-pictures")), i18nc("@action: context menu", "Album"), this);
    d->gotoDateAction     = new QAction(QIcon::fromTheme(QLatin1String("view-calendar")),   i18nc("@action: context menu", "Date"),  this);
    QMenu* const gotoMenu = new QMenu(d->parent);
    gotoMenu->addAction(d->gotoAlbumAction);
    gotoMenu->addAction(d->gotoDateAction);

    TagsPopupMenu* const gotoTagsPopup = new TagsPopupMenu(d->selectedIds, TagsPopupMenu::DISPLAY, gotoMenu);
    QAction* const gotoTag             = gotoMenu->addMenu(gotoTagsPopup);
    gotoTag->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    gotoTag->setText(i18nc("@action: context menu", "Tag"));

    // Disable the goto Tag popup menu, if there are no tags at all.

    if (!CoreDbAccess().db()->hasTags(d->selectedIds))
    {
        gotoTag->setEnabled(false);
    }

    /**
     * TODO:tags to be ported to multiple selection
     */

    QList<Album*> albumList   = AlbumManager::instance()->currentAlbums();
    const Album* currentAlbum = nullptr;

    if (!albumList.isEmpty())
    {
        currentAlbum = albumList.first();
    }
    else
    {
        return;
    }

    if      (currentAlbum->type() == Album::PHYSICAL)
    {
        // If the currently selected album is the same as album to
        // which the image belongs, then disable the "Go To" Album.
        // (Note that in recursive album view these can be different).

        if (item.albumId() == currentAlbum->id())
        {
            d->gotoAlbumAction->setEnabled(false);
        }
    }
    else if (currentAlbum->type() == Album::DATE)
    {
        d->gotoDateAction->setEnabled(false);
    }

    QAction* const gotoMenuAction = gotoMenu->menuAction();
    gotoMenuAction->setIcon(QIcon::fromTheme(QLatin1String("go-jump")));
    gotoMenuAction->setText(i18nc("@action: context menu", "Go To"));

    connect(gotoTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalGotoTag(int)));

    addAction(gotoMenuAction);
}

void ContextMenuHelper::slotRemoveAllTags()
{
    if (d->selectedIds.isEmpty())
    {
        return;
    }

    ItemInfoList infos(d->selectedIds);

    MetadataRemover* const tool = new MetadataRemover(infos, MetadataRemover::Tags);
    tool->start();
}

} // namespace Digikam
