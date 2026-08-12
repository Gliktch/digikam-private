/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-05
 * Description : tags filter view
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagcheckview.h"

// Qt includes

#include <QMenu>
#include <QAction>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kselectaction.h>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagCheckView::Private
{
public:

    Private() = default;

public:

    const QString configToggleAutoTagsEntry             = QLatin1String("Toggle Auto Tags");

    TagCheckView::ToggleAutoTags toggleAutoTags         = TagCheckView::NoToggleAuto;
    bool                         checkNewTags           = false;

    KSelectAction*               toggleAutoAction       = nullptr;
    QAction*                     toggleNoneAction       = nullptr;
    QAction*                     toggleChildrenAction   = nullptr;
    QAction*                     toggleParentsAction    = nullptr;
    QAction*                     toggleBothAction       = nullptr;
};

// --------------------------------------------------------

TagCheckView::TagCheckView(QWidget* const parent, TagModel* const tagModel)
    : TagFolderView(parent, tagModel),
      d            (new Private)
{
    setSelectAlbumOnClick(false);
    setExpandOnSingleClick(false);
    setSelectOnContextMenu(false);
    setShowFindDuplicateAction(false);

    // prepare custom menu action

    d->toggleAutoAction     = new KSelectAction(i18n("Toggle Auto"), this);
    d->toggleNoneAction     = d->toggleAutoAction->addAction(i18nc("no auto toggle", "None"));
    d->toggleAutoAction->menu()->addSeparator();
    d->toggleChildrenAction = d->toggleAutoAction->addAction(i18nc("toggle child tags", "Children"));
    d->toggleParentsAction  = d->toggleAutoAction->addAction(i18nc("toggle parent tag", "Parents"));
    d->toggleBothAction     = d->toggleAutoAction->addAction(i18nc("toggle child and parent tags", "Both"));

    d->toggleNoneAction->setData(NoToggleAuto);
    d->toggleChildrenAction->setData(Children);
    d->toggleParentsAction->setData(Parents);
    d->toggleBothAction->setData(ChildrenAndParents);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    connect(d->toggleAutoAction, SIGNAL(actionTriggered(QAction*)),
            this, SLOT(toggleAutoActionSelected(QAction*)));

#else

    connect(d->toggleAutoAction, SIGNAL(triggered(QAction*)),
            this, SLOT(toggleAutoActionSelected(QAction*)));

#endif

    connect(albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotCheckStateChange(Album*,Qt::CheckState)));
}

TagCheckView::~TagCheckView()
{
    delete d;
}

void TagCheckView::slotResetCheckState()
{
    checkableAlbumModel()->resetAllCheckedAlbums();
}

void TagCheckView::slotCheckStateChange(Album* album, Qt::CheckState state)
{
    Q_UNUSED(album);
    Q_UNUSED(state);

    // handle custom toggle modes

    disconnect(albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
               this, SLOT(slotCheckStateChange(Album*,Qt::CheckState)));

    // avoid signal recursion here

    switch (d->toggleAutoTags)
    {
        case Children:
        {
            checkableAlbumModel()->setCheckState(album, state);
            checkableAlbumModel()->setCheckStateForChildren(album, state);
            break;
        }

        case Parents:
        {
            checkableAlbumModel()->setCheckState(album, state);
            checkableAlbumModel()->setCheckStateForParents(album, state);
            break;
        }

        case ChildrenAndParents:
        {
            checkableAlbumModel()->setCheckState(album, state);
            checkableAlbumModel()->setCheckStateForChildren(album, state);
            checkableAlbumModel()->setCheckStateForParents(album, state);
            break;
        }

        default:
        {
            break;
        }
    }

    connect(albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotCheckStateChange(Album*,Qt::CheckState)));

    Q_EMIT checkedTagsChanged(getCheckedTags(), getPartiallyCheckedTags());
}

void TagCheckView::doLoadState()
{
    TagFolderView::doLoadState();

    KConfigGroup group = getConfigGroup();
    d->toggleAutoTags  = (ToggleAutoTags)
                         (group.readEntry(entryName(d->configToggleAutoTagsEntry), (int)NoToggleAuto));
}

void TagCheckView::doSaveState()
{
    TagFolderView::doSaveState();

    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->configToggleAutoTagsEntry), (int)(d->toggleAutoTags));
    group.sync();
}

QList<TAlbum*> TagCheckView::getCheckedTags() const
{
    QList<TAlbum*> tags;
    const auto albums = checkableAlbumModel()->checkedAlbums();

    for (Album* const album : albums)                                 // cppcheck-suppress constVariablePointer
    {
        TAlbum* const tag = dynamic_cast<TAlbum*> (album);            // cppcheck-suppress constVariablePointer

        if (tag)
        {
            tags << tag;
        }
    }

    return tags;
}

QList<TAlbum*> TagCheckView::getPartiallyCheckedTags() const
{
    QList<TAlbum*> tags;
    const auto albums = checkableAlbumModel()->partiallyCheckedAlbums();

    for (Album* const album : albums)                                 // cppcheck-suppress constVariablePointer
    {
        TAlbum* const tag = dynamic_cast<TAlbum*> (album);            // cppcheck-suppress constVariablePointer

        if (tag)
        {
            tags << tag;
        }
    }

    return tags;
}

TagCheckView::ToggleAutoTags TagCheckView::getToggleAutoTags() const
{
    return d->toggleAutoTags;
}

void TagCheckView::setToggleAutoTags(TagCheckView::ToggleAutoTags toggle)
{
    d->toggleAutoTags = toggle;
}

void TagCheckView::setCheckNewTags(bool checkNewTags)
{
    if (d->checkNewTags == checkNewTags)
    {
        return;
    }

    d->checkNewTags = checkNewTags;

    if (d->checkNewTags)
    {
        connect(tagModificationHelper(), SIGNAL(tagCreated(TAlbum*)),
                this, SLOT(slotCreatedNewTagByContextMenu(TAlbum*)));
    }
    else
    {
        disconnect(tagModificationHelper(), SIGNAL(tagCreated(TAlbum*)),
                   this, SLOT(slotCreatedNewTagByContextMenu(TAlbum*)));
    }
}

bool TagCheckView::checkNewTags() const
{
    return d->checkNewTags;
}

void TagCheckView::slotCreatedNewTagByContextMenu(TAlbum* tag)
{
    checkableAlbumModel()->setChecked(tag, true);

    if (albumFilterModel())
    {
        albumFilterModel()->updateFilter();
    }
}

void TagCheckView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    TagFolderView::addCustomContextMenuActions(cmh, album);

    cmh.addSeparator();

    // selection (checked) modification

    cmh.setAlbumModel(checkableAlbumModel());
    cmh.addAlbumCheckUncheckActions(album);
    cmh.addSeparator();

    // automatic toggle

    cmh.addAction(d->toggleAutoAction);
    const auto acs = d->toggleAutoAction->actions();

    for (QAction* const action : acs)
    {
        if (action->data().toInt() == d->toggleAutoTags)
        {
            action->setChecked(true);
        }
    }
}

void TagCheckView::toggleAutoActionSelected(QAction* action)
{
    d->toggleAutoTags = static_cast<ToggleAutoTags>(action->data().toInt());
}

} // namespace Digikam

#include "moc_tagcheckview.cpp"
