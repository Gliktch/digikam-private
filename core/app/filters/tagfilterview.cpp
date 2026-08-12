/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : tag filter view for the right sidebar
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagfilterview.h"

// Qt includes

#include <QAction>

// KDE includes

#include <klocalizedstring.h>
#include <kselectaction.h>

// Local includes

#include "digikam_debug.h"
#include "albummodel.h"
#include "contextmenuhelper.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagFilterView::Private
{
public:

    Private() = default;

public:

    QAction*       onRestoreTagFiltersAction    = nullptr;
    QAction*       offRestoreTagFiltersAction   = nullptr;
    QAction*       ignoreTagAction              = nullptr;
    QAction*       includeTagAction             = nullptr;
    QAction*       excludeTagAction             = nullptr;

    KSelectAction* restoreTagFiltersAction      = nullptr;
    KSelectAction* tagFilterModeAction          = nullptr;

    TagModel*      tagFilterModel               = nullptr;
};

TagFilterView::TagFilterView(QWidget* const parent, TagModel* const tagFilterModel)
    : TagCheckView(parent, tagFilterModel),
      d           (new Private)
{
    d->tagFilterModel             = tagFilterModel;

    d->restoreTagFiltersAction    = new KSelectAction(i18nc("@action: tag filter", "Restore Tag Filters"), this);
    d->onRestoreTagFiltersAction  = d->restoreTagFiltersAction->addAction(i18nc("@action: tag filter", "On"));
    d->offRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18nc("@action: tag filter", "Off"));

    d->tagFilterModeAction        = new KSelectAction(i18nc("@action: tag filter", "Tag Filter Mode"), this);
    d->ignoreTagAction            = d->tagFilterModeAction->addAction(i18nc("@action: tag filter", "Ignore This Tag"));
    d->includeTagAction           = d->tagFilterModeAction->addAction(QIcon::fromTheme(QLatin1String("list-add")),    i18nc("@action: tag filter", "Must Have This Tag"));
    d->excludeTagAction           = d->tagFilterModeAction->addAction(QIcon::fromTheme(QLatin1String("list-remove")), i18nc("@action: tag filter", "Must Not Have This Tag"));

    connect(tagModificationHelper(), SIGNAL(aboutToDeleteTag(TAlbum*)),
            this, SLOT(slotDeleteTagByContextMenu(TAlbum*)));
}

TagFilterView::~TagFilterView()
{
    delete d;
}

void TagFilterView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    TagCheckView::addCustomContextMenuActions(cmh, album);

    // restoring

    cmh.addAction(d->restoreTagFiltersAction);

    Qt::CheckState state = d->tagFilterModel->checkState(album);

    switch (state)
    {
        case Qt::Unchecked:
        {
            d->tagFilterModeAction->setCurrentAction(d->ignoreTagAction);
            break;
        }

        case Qt::PartiallyChecked:
        {
            d->tagFilterModeAction->setCurrentAction(d->excludeTagAction);
            break;
        }

        case Qt::Checked:
        {
            d->tagFilterModeAction->setCurrentAction(d->includeTagAction);
            break;
        }
    }

    cmh.addAction(d->tagFilterModeAction);

    d->onRestoreTagFiltersAction->setChecked(isRestoreCheckState());
    d->offRestoreTagFiltersAction->setChecked(!isRestoreCheckState());
}

void TagFilterView::handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album)
{
    TagCheckView::handleCustomContextMenuAction(action, album);

    if (!action)
    {
        return;
    }

    if      (action == d->onRestoreTagFiltersAction)        // Restore TagFilters ON.
    {
        setRestoreCheckState(true);
    }
    else if (action == d->offRestoreTagFiltersAction)       // Restore TagFilters OFF.
    {
        setRestoreCheckState(false);
    }
    else if (action == d->ignoreTagAction)
    {
        checkableAlbumModel()->setCheckState(album, Qt::Unchecked);
    }
    else if (action == d->includeTagAction)
    {
        checkableAlbumModel()->setCheckState(album, Qt::Checked);
    }
    else if (action == d->excludeTagAction)
    {
        checkableAlbumModel()->setCheckState(album, Qt::PartiallyChecked);
    }
}

void TagFilterView::slotDeleteTagByContextMenu(TAlbum* tag)
{
    checkableAlbumModel()->setCheckState(tag, Qt::Unchecked);
}

} // namespace Digikam

#include "moc_tagfilterview.cpp"
