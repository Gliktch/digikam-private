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

void TagsManager::slotSelectionChanged()
{
    QList<Album*> selectedTags = d->tagMngrView->selectedTags();

    if (selectedTags.isEmpty() || ((selectedTags.size() == 1) && selectedTags.at(0)->isRoot()))
    {
        d->enableRootTagActions(false);
        d->listView->enableAddButton(false);
    }
    else
    {
        d->enableRootTagActions(true);
        d->listView->enableAddButton(true);
        d->titleEdit->setEnabled((selectedTags.size() == 1));
    }

    d->tagPropWidget->slotSelectionChanged(selectedTags);
}

void TagsManager::slotAddAction()
{
    TAlbum*      parent = d->tagMngrView->currentAlbum();
    QString      title;
    QString      icon;
    QKeySequence ks;

    if (!parent)
    {
        parent = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(d->tagMngrView->model()->index(0, 0)));
    }

    if (!TagEditDlg::tagCreate(qApp->activeWindow(), parent, title, icon, ks))
    {
        return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, ks, errMap);

    Q_UNUSED(tList);

    TagEditDlg::showtagsListCreationError(qApp->activeWindow(), errMap);
}

void TagsManager::slotDeleteAction()
{
    const QModelIndexList selected = d->tagMngrView->selectionModel()->selectedIndexes();

    QStringList tagNames;
    QStringList tagsWithChildren;
    QStringList tagsWithImages;
    QMultiMap<int, TAlbum*> sortedTags;

    for (const QModelIndex& index : std::as_const(selected))
    {
        if (!index.isValid())
        {
            return;
        }

        TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));

        if (!t || t->isRoot())
        {
            return;
        }

        AlbumPointer<TAlbum> tag(t);
        tagNames.append(tag->title());

        // find number of subtags

        int children = 0;
        AlbumIterator iter(tag);

        while (iter.current())
        {
            ++children;
            ++iter;
        }

        if (children)
        {
            tagsWithChildren.append(tag->title());
        }

        QList<qlonglong> assignedItems = CoreDbAccess().db()->getItemIDsInTag(tag->id());

        if (!assignedItems.isEmpty())
        {
            tagsWithImages.append(tag->title());
        }

        /**
         * Tags must be deleted from children to parents, if we don't want
         * to step on invalid index. Use QMultiMap to order them by distance
         * to root tag
         */
        const Album* parent = t;
        int depth           = 0;

        while (!parent->isRoot())
        {
            parent = parent->parent();
            depth++;
        }

        sortedTags.insert(depth, tag);
    }

    // ask for deletion of children

    if (!tagsWithChildren.isEmpty())
    {
        const int result = QMessageBox::warning(this, qApp->applicationName(),
                                                i18ncp("%2 is a comma separated list of tags to be deleted.",
                                                       "Tag %2 has one or more subtags. "
                                                       "Deleting it will also delete "
                                                       "the subtags. "
                                                       "Do you want to continue?",
                                                       "Tags %2 have one or more subtags. "
                                                       "Deleting them will also delete "
                                                       "the subtags. "
                                                       "Do you want to continue?",
                                                       tagsWithChildren.count(),
                                                       d->joinTagNamesToList(tagsWithChildren)),
                                                QMessageBox::Yes | QMessageBox::Cancel);

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    QString message;

    if (!tagsWithImages.isEmpty())
    {
        message = i18ncp("%2 is a comma separated list of tags to be deleted.",
                         "Tag %2 is assigned to one or more items. "
                         "Do you want to delete it?",
                         "Tags %2 are assigned to one or more items. "
                         "Do you want to delete them?",
                         tagsWithImages.count(),
                         d->joinTagNamesToList(tagsWithImages));
    }
    else
    {
        message = i18ncp("%2 is a comma separated list of tags to be deleted.",
                         "Delete tag %2?",
                         "Delete tags %2?",
                         tagNames.count(),
                         d->joinTagNamesToList(tagNames));
    }

    const int result = QMessageBox::warning(this, i18ncp("@title:window", "Delete tag", "Delete tags", tagNames.count()),
                                            message, QMessageBox::Yes | QMessageBox::Cancel);

    if (result == QMessageBox::Yes)
    {
        const QList<TAlbum*>& sortedTagsList = sortedTags.values();
        QList<TAlbum*>::const_reverse_iterator it;
        QList<qlonglong> imageIds;

        /**
         * QMultimap doesn't provide reverse iterator, use QList.
         */
        for (it = sortedTagsList.crbegin() ; it != sortedTagsList.crend() ; ++it)
        {
            QString errMsg;

            if (!AlbumManager::instance()->deleteTAlbum(*it, errMsg, &imageIds))
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
            }
        }

        AlbumManager::instance()->askUserForWriteChangedTAlbumToFiles(imageIds);
    }
}

void TagsManager::slotInvertSelection()
{
    QModelIndex root                 = d->tagMngrView->model()->index(0, 0);
    QItemSelectionModel* const model = d->tagMngrView->selectionModel();
    QModelIndexList selected         = model->selectedIndexes();

    QQueue<QModelIndex> greyNodes;
    bool currentSet = false;

    greyNodes.append(root);

    model->clearSelection();

    while (!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if (!(current.isValid()))
        {
            continue;
        }

        int it            = 0;
        QModelIndex child = current.model()->index(it++, 0, current);

        while (child.isValid())
        {
            if (!selected.contains(child))
            {
                if (!currentSet)
                {
                    /**
                     * Must set a new current item when inverting selection
                     * it should be done only once
                     */
                    d->tagMngrView->setCurrentIndex(child);
                    currentSet = true;
                }

                model->select(child, model->Select);
            }

            if (d->tagMngrView->isExpanded(child))
            {
                greyNodes.enqueue(child);
            }

            child = current.model()->index(it++, 0, current);
        }
    }
}

} // namespace Digikam
