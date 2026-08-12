/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * SPDX-FileCopyrightText:      2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsmanager_p.h"

namespace Digikam
{

void TagsManager::slotEditTagTitle()
{
    QList<Album*> selectedTags = d->tagMngrView->selectedTags();

    if ((selectedTags.size() == 1) && !selectedTags.at(0)->isRoot())
    {
        d->tagPropWidget->show();
        d->tagPropWidget->slotFocusTitleEdit();
    }
}

void TagsManager::slotTitleEditReady()
{
    if (!d->tagPropVisible)
    {
        d->tagPropWidget->hide();
    }

    d->tagMngrView->setFocus();
}

void TagsManager::slotResetTagIcon()
{
    QString errMsg;

    const QList<TAlbum*> selected = d->tagMngrView->selectedTagAlbums();

    for (QList<TAlbum*>::const_iterator it = selected.constBegin() ; it != selected.constEnd() ; ++it)
    {
        TAlbum* const tag = *it;

        if (tag)
        {
            if (!AlbumManager::instance()->updateTAlbumIcon(tag, tag->standardIconName(), 0, errMsg))
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
            }
        }
    }
}

void TagsManager::slotMarkNotAssignedTags()
{
    QModelIndex root = d->tagMngrView->model()->index(0, 0);

    QQueue<QModelIndex> greyNodes;
    QList<QModelIndex>  redNodes;
    QSet<QModelIndex>   greenNodes;

    int iter = 0;

    while (root.model()->hasIndex(iter, 0, root))
    {
        greyNodes.append(root.model()->index(iter++, 0, root));
    }

    while (!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if (!(current.isValid()))
        {
            continue;
        }

        if (current.model()->hasIndex(0, 0, current))
        {
            // Add in the list

            int iterator = 0;

            while (current.model()->hasIndex(iterator, 0, current))
            {
                greyNodes.append(current.model()->index(iterator++, 0, current));
            }
        }
        else
        {
            const TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(current));

            if (t && !t->isRoot() && !t->isInternalTag())
            {
                QList<qlonglong> assignedItems = CoreDbAccess().db()->getItemIDsInTag(t->id());

                if (assignedItems.isEmpty())
                {
                    redNodes.append(current);
                }
                else
                {
                    QModelIndex tmp = current.parent();

                    while (tmp.isValid())
                    {
                        greenNodes.insert(tmp);
                        tmp = tmp.parent();
                    }
                }
            }
        }
    }

    QItemSelectionModel* const model = d->tagMngrView->selectionModel();
    model->clearSelection();
    QList<int> toMark;

    for (const QModelIndex& index : std::as_const(redNodes))
    {
        QModelIndex current = index;

        while (current.isValid() && !greenNodes.contains(current))
        {
            const TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(current));

            if (
                t                                       &&
                !t->isRoot()                            &&
                !t->isInternalTag()                     &&
                !FaceTags::isSystemPersonTagId(t->id()) &&
                (FaceTags::personParentTag() != t->id())
               )
            {
                QList<qlonglong> assignedItems = CoreDbAccess().db()->getItemIDsInTag(t->id());

                if (assignedItems.isEmpty() && !toMark.contains(t->id()))
                {
                    model->select(current, model->Select);
                    toMark.append(t->id());
                }
                else
                {
                    break;
                }
            }

            current = current.parent();
        }
    }
}

} // namespace Digikam
