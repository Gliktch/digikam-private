/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Basic search tree view with editing functionality
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "editablesearchtreeview.h"

// Qt includes

#include <QAction>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN EditableSearchTreeView::Private
{
public:

    Private() = default;

public:

    SearchModificationHelper* searchModificationHelper  = nullptr;

    QAction*                  renameSearchAction        = nullptr;
    QAction*                  deleteSearchAction        = nullptr;
};

EditableSearchTreeView::EditableSearchTreeView(QWidget* const parent,
                                               SearchModel* const searchModel,
                                               SearchModificationHelper* const searchModificationHelper)
    : SearchTreeView(parent),
      d             (new Private)
{
    setSAlbumModel(searchModel);
    d->searchModificationHelper = searchModificationHelper;

    d->renameSearchAction       = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Rename..."), this);
    d->deleteSearchAction       = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),   i18n("Delete"),    this);

    setSortingEnabled(true);
    setSelectAlbumOnClick(true);
    setEnableContextMenu(true);
}

EditableSearchTreeView::~EditableSearchTreeView()
{
    delete d;
}

QString EditableSearchTreeView::contextMenuTitle() const
{
    return i18n("Searches");
}

void EditableSearchTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    const SAlbum* const searchAlbum = dynamic_cast<SAlbum*>(album);

    // disable actions if there is no album or the album is a temporary search

    bool activate = false;

    if (searchAlbum)
    {
        activate = !searchAlbum->isTemporarySearch();
        d->deleteSearchAction->setEnabled(true);
    }

    d->renameSearchAction->setEnabled(activate);

    cmh.addAction(d->renameSearchAction);
    cmh.addAction(d->deleteSearchAction);
}

void EditableSearchTreeView::handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album)
{
    Album* const a            = album;
    SAlbum* const searchAlbum = dynamic_cast<SAlbum*>(a);

    if (!searchAlbum || !action)
    {
        return;
    }

    if      (action == d->renameSearchAction)
    {
        d->searchModificationHelper->slotSearchRename(searchAlbum);
    }
    else if (action == d->deleteSearchAction)
    {
        d->searchModificationHelper->slotSearchDelete(searchAlbum);
    }
}

} // namespace Digikam

#include "moc_editablesearchtreeview.cpp"
