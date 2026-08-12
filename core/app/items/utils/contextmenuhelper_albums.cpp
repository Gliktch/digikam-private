/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class - Albums methods.
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

void ContextMenuHelper::addActionNewAlbum(AlbumModificationHelper* const helper, PAlbum* const parentAlbum)
{
    QAction* const action = d->copyFromMainCollection(QLatin1String("album_new"));
    addAction(action);
    helper->bindAlbum(action, parentAlbum);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumNew()));
}

void ContextMenuHelper::addActionDeleteAlbum(AlbumModificationHelper* const helper, PAlbum* const album)
{
    QAction* const action = d->copyFromMainCollection(QLatin1String("album_delete"));
    addAction(action, !(album->isRoot() || album->isAlbumRoot()));
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumDelete()));
}

void ContextMenuHelper::addActionEditAlbum(AlbumModificationHelper* const helper, PAlbum* const album)
{
    QAction* const action = d->copyFromMainCollection(QLatin1String("album_propsEdit"));
    addAction(action, !album->isRoot());
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumEdit()));
}

void ContextMenuHelper::addActionRenameAlbum(AlbumModificationHelper* const helper, PAlbum* const album)
{
    QAction* const action = d->copyFromMainCollection(QLatin1String("album_rename"));
    addAction(action, !album->isRoot());
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumRename()));
}

void ContextMenuHelper::addActionResetAlbumIcon(AlbumModificationHelper* const helper, PAlbum* const album)
{
    QAction* const action = new QAction(QIcon::fromTheme(QLatin1String("run-build-prune")),
                                        i18nc("@action: context menu", "Reset Album Icon"), this);
    addAction(action, !album->isRoot());
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumResetIcon()));
}

void ContextMenuHelper::slotDeselectAllAlbumItems()
{
    QAction* const selectNoneAction = d->stdActionCollection->action(QLatin1String("selectNone"));
    QTimer::singleShot(75, selectNoneAction, SIGNAL(triggered()));
}

void ContextMenuHelper::setAlbumModel(AbstractCheckableAlbumModel* model)
{
    d->albumModel = model;
}

void ContextMenuHelper::addAlbumCheckUncheckActions(Album* const album)
{
    bool     enabled   = false;
    QString  allString = i18nc("@action: context menu", "All Albums");
    QVariant albumData;

    if (album)
    {
        enabled   = true;
        albumData = QVariant::fromValue(AlbumPointer<>(album));

        if (album->type() == Album::TAG)
        {
            allString = i18nc("@action: context menu", "All Tags");
        }
    }

    QMenu* const selectTagsMenu         = new QMenu(i18nc("@action: select tags menu", "Select"));
    addSubMenu(selectTagsMenu);

    selectTagsMenu->addAction(allString, d->albumModel, SLOT(checkAllAlbums()));
    selectTagsMenu->addSeparator();
    QAction* const selectChildrenAction = selectTagsMenu->addAction(i18nc("@action: context menu", "Children"), this, SLOT(slotSelectChildren()));
    QAction* const selectParentsAction  = selectTagsMenu->addAction(i18nc("@action: context menu", "Parents"),  this, SLOT(slotSelectParents()));
    selectChildrenAction->setData(albumData);
    selectParentsAction->setData(albumData);

    QMenu* const deselectTagsMenu       = new QMenu(i18nc("@action: deselect tags menu", "Deselect"));
    addSubMenu(deselectTagsMenu);

    deselectTagsMenu->addAction(allString, d->albumModel, SLOT(resetAllCheckedAlbums()));
    deselectTagsMenu->addSeparator();
    QAction* const deselectChildrenAction = deselectTagsMenu->addAction(i18nc("@action: context menu", "Children"), this, SLOT(slotDeselectChildren()));
    QAction* const deselectParentsAction  = deselectTagsMenu->addAction(i18nc("@action: context menu", "Parents"),  this, SLOT(slotDeselectParents()));
    deselectChildrenAction->setData(albumData);
    deselectParentsAction->setData(albumData);

    d->parent->addAction(i18nc("@action: context menu", "Invert Selection"), d->albumModel, SLOT(invertCheckedAlbums()));

    selectChildrenAction->setEnabled(enabled);
    selectParentsAction->setEnabled(enabled);
    deselectChildrenAction->setEnabled(enabled);
    deselectParentsAction->setEnabled(enabled);
}

void ContextMenuHelper::slotSelectChildren()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->checkAllAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::slotDeselectChildren()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->resetCheckedAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::slotSelectParents()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->checkAllParentAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::slotDeselectParents()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->resetCheckedParentAlbums(d->indexForAlbumFromAction(sender()));
}

} // namespace Digikam
