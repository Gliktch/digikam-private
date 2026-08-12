/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - View methods.
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

void ItemIconView::setHostWindowActions(const HostActionsMap& actions)
{
    d->stackedView->imagePreviewView()->setHostWindowActions(actions);

#ifdef HAVE_MEDIAPLAYER

    d->stackedView->mediaPlayerView()->setHostWindowActions(actions);

#endif // HAVE_MEDIAPLAYER

}

void ItemIconView::connectIconViewFilter(FilterStatusBar* const filterbar)
{
    ItemAlbumFilterModel* const model = d->iconView->itemAlbumFilterModel();

    connect(model, SIGNAL(filterMatches(bool)),
            filterbar, SLOT(slotFilterMatches(bool)));

    connect(model, SIGNAL(filterSettingsChanged(ItemFilterSettings)),
            filterbar, SLOT(slotFilterSettingsChanged(ItemFilterSettings)));

    connect(filterbar, SIGNAL(signalResetFilters()),
            d->filterWidget, SLOT(slotResetFilters()));

    connect(filterbar, SIGNAL(signalPopupFiltersView()),
            this, SLOT(slotPopupFiltersView()));
}

void ItemIconView::slotEscapePreview()
{
    if (
        (viewMode() == StackedView::IconViewMode)  ||
        (viewMode() == StackedView::MapWidgetMode) ||
        (viewMode() == StackedView::TableViewMode) ||
        (viewMode() == StackedView::TrashViewMode) ||
        (viewMode() == StackedView::WelcomePageMode)
       )
    {
        return;
    }

    // pass a null image info, because we want to fall back to the old view mode

    slotTogglePreviewMode(ItemInfo());
}

void ItemIconView::slotMapWidgetView()
{
    d->stackedView->setViewMode(StackedView::MapWidgetMode, true);
}

void ItemIconView::slotTableView()
{
    d->stackedView->setViewMode(StackedView::TableViewMode, true);
}

void ItemIconView::slotIconView()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }

    // and switch to icon view

    d->stackedView->setViewMode(StackedView::IconViewMode, true);

    // make sure the next/previous buttons are updated

    slotImageSelected();
}

void ItemIconView::slotImagePreview()
{
    slotTogglePreviewMode(currentInfo());
}

/**
 * @brief This method toggles between AlbumView/MapWidgetView and ImagePreview modes, depending on the context.
 */
void ItemIconView::slotTogglePreviewMode(const ItemInfo& info)
{
    if (
        (viewMode() == StackedView::IconViewMode)  ||
        (viewMode() == StackedView::TableViewMode) ||
        (viewMode() == StackedView::MapWidgetMode)
       )
    {
        d->lastViewMode = viewMode();

        if (!info.isNull())
        {
            if (info.isLocationAvailable())
            {
                if (viewMode() == StackedView::IconViewMode)
                {
                    d->stackedView->setPreviewItem(info,
                                                   d->iconView->previousInfo(info),
                                                   d->iconView->nextInfo(info));
                }
                else
                {
                    d->stackedView->setPreviewItem(info,
                                                   ItemInfo(),
                                                   ItemInfo());
                }
            }
            else
            {
                QModelIndex index = d->iconView->indexForInfo(info);
                d->iconView->showIndexNotification(index,
                                                   i18nc("@info: item icon view",
                                                         "The storage location of this image\n"
                                                         "is currently not available"));
            }
        }
    }
    else
    {
        // go back to the last AlbumViewMode

        d->stackedView->setViewMode(d->lastViewMode);
    }

    // make sure the next/previous buttons are updated

    slotImageSelected();
}

void ItemIconView::slotViewModeChanged()
{
    toggleZoomActions();

    switch (viewMode())
    {
        case StackedView::IconViewMode:
        {
            Q_EMIT signalSwitchedToIconView();
            Q_EMIT signalThumbSizeChanged(d->thumbSize);

            break;
        }

        case StackedView::PreviewImageMode:
        {
            Q_EMIT signalSwitchedToPreview();

            slotZoomFactorChanged(d->stackedView->zoomFactor());

            break;
        }

        case StackedView::WelcomePageMode:
        {
            Q_EMIT signalSwitchedToIconView();

            break;
        }

        case StackedView::MediaPlayerMode:
        {
            Q_EMIT signalSwitchedToPreview();

            break;
        }

        case StackedView::MapWidgetMode:
        {
            Q_EMIT signalSwitchedToMapView();

            // TODO: connect map view's zoom buttons to main status bar zoom buttons

            break;
        }

        case StackedView::TableViewMode:
        {
            Q_EMIT signalSwitchedToTableView();
            Q_EMIT signalThumbSizeChanged(d->thumbSize);

            break;
        }

        case StackedView::TrashViewMode:
        {
            d->msgNotifyTimer->stop();
            d->errorWidget->animatedHide();

            Q_EMIT signalSwitchedToTrashView();

            break;
        }
    }
}

void ItemIconView::toggleShowBar(bool b)
{
    d->stackedView->thumbBarDock()->showThumbBar(b);

    // See bug #319876 : force to reload current view mode to set thumbbar visibility properly.

    d->stackedView->setViewMode(viewMode());
}

StackedView::StackedViewMode ItemIconView::viewMode() const
{
    return d->stackedView->viewMode();
}

void ItemIconView::slotSetupMetadataFilters(int tab)
{
    Setup::execMetadataFilters(this, tab);
}

void ItemIconView::slotSetupExifTool()
{
    Setup::execExifTool(this);
}

void ItemIconView::toggleFullScreen(bool set)
{
    d->stackedView->imagePreviewView()->toggleFullScreen(set);
}

void ItemIconView::setToolsIconView(DCategorizedView* const view)
{
    d->rightSideBar->appendTab(view,
                               QIcon::fromTheme(QLatin1String("document-edit")),
                               i18nc("@title: item icon view", "Tools"));
}

void ItemIconView::refreshView()
{
    d->rightSideBar->refreshTagsView();
}

void ItemIconView::slotShowContextMenu(QContextMenuEvent* event,
                                       const QList<QAction*>& extraGroupingActions)
{
    const Album* const album = currentAlbum();

    if (
        !album          ||
        album->isRoot() ||
        (
         (album->type() != Album::PHYSICAL) &&
         (album->type() != Album::TAG)
        )
       )
    {
        return;
    }

    QMenu menu(this);
    ContextMenuHelper cmHelper(&menu);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();
    cmHelper.addStandardActionPaste(this, SLOT(slotImagePaste()));

    if (!extraGroupingActions.isEmpty())
    {
        cmHelper.addSeparator();
        cmHelper.addGroupMenu(QList<qlonglong>(), extraGroupingActions);
    }

    cmHelper.exec(event->globalPos());
}

void ItemIconView::slotShowContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info,
                                             const QList<QAction*>& extraGroupingActions,
                                             ItemFilterModel* imageFilterModel)
{
    QList<qlonglong> selectedImageIds = selectedInfoList(true, true).toImageIdList();

    // --------------------------------------------------------

    QMenu menu(this);
    ContextMenuHelper cmHelper(&menu);
    cmHelper.setItemFilterModel(imageFilterModel);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    QAction* const viewAction = new QAction(i18nc("@action: View the selected image", "Preview"), this);
    viewAction->setIcon(QIcon::fromTheme(QLatin1String("view-preview")));
    viewAction->setEnabled(selectedImageIds.count() == 1);
    cmHelper.addAction(viewAction);

    cmHelper.addOpenAndNavigateActions(selectedImageIds);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    QMenu* const fmenu = new QMenu(i18nc("@action: face workflow", "Faces"));
    fmenu->setIcon(QIcon::fromTheme(QLatin1String("edit-image-face-show")));
    fmenu->addAction(DigikamApp::instance()->actionCollection()->action(QLatin1String("image_scan_for_faces")));
    fmenu->addAction(DigikamApp::instance()->actionCollection()->action(QLatin1String("image_recognize_faces")));
    fmenu->addAction(DigikamApp::instance()->actionCollection()->action(QLatin1String("image_remove_all_faces")));
    cmHelper.addSubMenu(fmenu);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_find_similar"));
    cmHelper.addStandardActionLightTable();
    cmHelper.addQueueManagerMenu();
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_rotate"));
    cmHelper.addAction(QLatin1String("cut_album_selection"));
    cmHelper.addAction(QLatin1String("copy_album_selection"));
    cmHelper.addAction(QLatin1String("paste_album_selection"));
    cmHelper.addAction(QLatin1String("image_rename"));
    cmHelper.addStandardActionItemDelete(this, SLOT(slotImageDelete()), selectedImageIds.count());
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addIQSAction(this, SLOT(slotImageQualitySorter()));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addStandardActionThumbnail(selectedImageIds, currentAlbum());
    cmHelper.addAssignTagsMenu(selectedImageIds);
    cmHelper.addRemoveTagsMenu(selectedImageIds);
    cmHelper.addRemoveAllTags(selectedImageIds);
    cmHelper.addLabelsAction();

    if (d->leftSideBar->getActiveTab() != d->peopleSideBar)
    {
        cmHelper.addSeparator();

        cmHelper.addGroupMenu(selectedImageIds, extraGroupingActions);
    }

    // special action handling --------------------------------

    connect(&cmHelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmHelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmHelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmHelper, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            d->albumHistory, SLOT(slotClearSelectTAlbum(int)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ItemInfo)),
            d->albumHistory, SLOT(slotClearSelectPAlbum(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalGotoDate(ItemInfo)),
            this, SLOT(slotGotoDateAndItem(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalSetThumbnail(ItemInfo)),
            this, SLOT(slotSetAsAlbumThumbnail(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(&cmHelper, SIGNAL(signalCreateGroup()),
            this, SLOT(slotCreateGroupFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(slotCreateGroupByTimeFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByFilename()),
            this, SLOT(slotCreateGroupByFilenameFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByTimelapse()),
            this, SLOT(slotCreateGroupByTimelapseFromSelection()));

    connect(&cmHelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(slotRemoveSelectedFromGroup()));

    connect(&cmHelper, SIGNAL(signalUngroup()),
            this, SLOT(slotUngroupSelected()));

    // --------------------------------------------------------

    const QAction* const choice = cmHelper.exec(event->globalPos());

    if (choice && (choice == viewAction))
    {
        slotTogglePreviewMode(info);
    }
}

void ItemIconView::slotShowGroupContextMenu(QContextMenuEvent* event,
                                            const QList<ItemInfo>& selectedInfos,
                                            ItemFilterModel* imageFilterModel)
{
    QList<qlonglong> selectedImageIDs;

    for (const ItemInfo& info : std::as_const(selectedInfos))
    {
        selectedImageIDs << info.id();
    }

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setItemFilterModel(imageFilterModel);
    cmhelper.addGroupActions(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(slotCreateGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(slotCreateGroupByTimeFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByFilename()),
            this, SLOT(slotCreateGroupByFilenameFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTimelapse()),
            this, SLOT(slotCreateGroupByTimelapseFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(slotUngroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(slotRemoveSelectedFromGroup()));

    cmhelper.exec(event->globalPos());
}

} // namespace Digikam
