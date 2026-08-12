/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-05
 * Description : Tag Manager Tree View derived from TagsFolderView to implement
 *               a custom context menu and some batch view options, such as
 *               expanding multiple items
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagmngrtreeview.h"

// Qt includes

#include <QModelIndex>
#include <QQueue>
#include <QMenu>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "tagsmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagMngrTreeView::Private
{

public:

    Private() = default;

public:

    TagsManager* tagMngr = nullptr;
};

TagMngrTreeView::TagMngrTreeView(TagsManager* const parent, TagModel* const model)
    : TagFolderView(parent, model),
      d            (new Private)
{
    d->tagMngr = parent;
    setAlbumFilterModels(new TagsManagerFilterModel(this), checkableAlbumFilterModel());
    setSelectAlbumOnClick(false);
    expand(albumFilterModel()->rootAlbumIndex());
}

TagMngrTreeView::~TagMngrTreeView()
{
    delete d;
}

void TagMngrTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndexList selectedItems = selectionModel()->selectedIndexes();

    std::sort(selectedItems.begin(), selectedItems.end());
    QList<TAlbum*> items;

    for (const QModelIndex& mIndex : std::as_const(selectedItems))
    {
        TAlbum* const temp = static_cast<TAlbum*>(albumForIndex(mIndex));
        items.append(temp);
    }

    /**
     * Append root tag if no nodes are selected
     */
    if (items.isEmpty())
    {
        QModelIndex root = model()->index(0, 0);
        items.append(static_cast<TAlbum*>(albumForIndex(root)));
    }

    QMenu popmenu(this);
    popmenu.addSection(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(&popmenu);
    setContexMenuItems(cmhelper, items);

    QAction* const choice = cmhelper.exec(QCursor::pos());

    Q_UNUSED(choice);
    Q_UNUSED(event);
}

void TagMngrTreeView::setAlbumFilterModels(TagsManagerFilterModel* const filteredModel,
                                           CheckableAlbumFilterModel* const filterModel)
{
    Q_UNUSED(filterModel);

    m_tfilteredModel = filteredModel;
    albumFilterModel()->setSourceFilterModel(m_tfilteredModel);
}

void TagMngrTreeView::setContexMenuItems(ContextMenuHelper& cmh, const QList<TAlbum*>& albums)
{
    bool isRoot = false;

    if (albums.size() == 1)
    {
        TAlbum* const tag = dynamic_cast<TAlbum*> (albums.first());

        if (!tag)
        {
            return;
        }

        if (tag->isRoot())
        {
            isRoot = true;
        }

        cmh.addActionNewTag(tagModificationHelper(), tag);
    }

    if (!isRoot)
    {
        cmh.addActionDeleteTags(tagModificationHelper(), albums);
    }
    else
    {
        /**
         * This is a dummy action, delete is disable for root tag
         */
        QAction* deleteTagsAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                i18n("Delete Tags"), this);
        cmh.addAction(deleteTagsAction);
        deleteTagsAction->setEnabled(false);
    }

    cmh.addSeparator();

    QAction* const titleEdit     = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                               i18n("Edit Tag Title"), this);
    titleEdit->setShortcut(QKeySequence(Qt::Key_F2));

    QAction* const resetIcon     = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                                               i18n("Reset Tag Icon"), this);

    QAction* const invSel        = new QAction(QIcon::fromTheme(QLatin1String("tag-reset")),
                                               i18n("Invert Selection"), this);

    QAction* const expandSel     = new QAction(QIcon::fromTheme(QLatin1String("go-down")),
                                               i18n("Expand Selected Nodes"), this);

    QAction* const expandAll     = new QAction(QIcon::fromTheme(QLatin1String("expand-all")),
                                               i18n("Expand Tag Tree"), this);

    QAction* const collapseAll   = new QAction(QIcon::fromTheme(QLatin1String("collapse-all")),
                                               i18n("Collapse Tag Tree"), this);

    QAction* const delTagFromImg = new QAction(QIcon::fromTheme(QLatin1String("tag-delete")),
                                               i18n("Remove Tag from Images"), this);

    cmh.addAction(titleEdit,     d->tagMngr, SLOT(slotEditTagTitle()),         false);
    cmh.addAction(resetIcon,     d->tagMngr, SLOT(slotResetTagIcon()),         false);
    cmh.addAction(invSel,        d->tagMngr, SLOT(slotInvertSelection()),      false);
    cmh.addAction(expandSel,     this ,      SLOT(slotExpandNode()),           false);
    cmh.addAction(expandAll,     this,       SLOT(expandAll()),                false);
    cmh.addAction(collapseAll,   this,       SLOT(slotCollapseAllNodes()),     false);
    cmh.addAction(delTagFromImg, d->tagMngr, SLOT(slotRemoveTagsFromImages()), false);

    if (isRoot)
    {
        titleEdit->setEnabled(false);
        resetIcon->setEnabled(false);
        delTagFromImg->setEnabled(false);
    }

    if (albums.size() != 1)
    {
        titleEdit->setEnabled(false);
    }
}

} // namespace Digikam

#include "moc_tagmngrtreeview.cpp"
