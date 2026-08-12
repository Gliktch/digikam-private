/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - Item methods.
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

QList<QUrl> ItemIconView::allUrls(bool grouping) const
{
    /// @todo This functions seems not to be used anywhere right now

    return allInfo(grouping).toImageUrlList();
}

QList<QUrl> ItemIconView::selectedUrls(bool grouping) const
{
    return selectedInfoList(false, grouping).toImageUrlList();
}

QList<QUrl> ItemIconView::selectedUrls(const OperationType type) const
{
    return selectedInfoList(type).toImageUrlList();
}

void ItemIconView::slotFirstItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotGoToRow(0, false);
            break;
        }

        default:
        {
            // all other views are tied to IconView's selection model

            d->iconView->toFirstIndex();
            break;
        }
    }
}

void ItemIconView::slotPrevItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotGoToRow(-1, true);
            break;
        }

        default:
        {
            // all other views are tied to IconView's selection model

            d->iconView->toPreviousIndex();
            break;
        }
    }
}

void ItemIconView::slotNextItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotGoToRow(1, true);
            break;
        }

        default:
        {
            // all other views are tied to IconView's selection model

            d->iconView->toNextIndex();
            break;
        }
    }
}

void ItemIconView::slotLastItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotGoToRow(-1, false);
            break;
        }

        default:
        {
            // all other views are tied to IconView's selection model

            d->iconView->toLastIndex();
            break;
        }
    }
}

void ItemIconView::slotSelectItemByUrl(const QUrl& url)
{
    /// @todo This functions seems not to be used anywhere right now
    /// @todo Adapt to TableView

    d->iconView->toIndex(url);
}

void ItemIconView::slotImageSelected()
{
    // delay to slotDispatchImageSelected

    d->selectionTimer->start();

    switch (viewMode())
    {
        case StackedView::TrashViewMode:
        {
            Q_EMIT signalTrashSelectionChanged(d->trashView->statusBarText());

            break;
        }

        case StackedView::TableViewMode:
        {
            Q_EMIT signalSelectionChanged(d->tableView->numberOfSelectedItems());

            break;
        }

        default:
        {
            Q_EMIT signalSelectionChanged(d->iconView->numberOfSelectedIndexes());

            break;
        }
    }
}

void ItemIconView::slotDispatchImageSelected()
{
    if (viewMode() == StackedView::TrashViewMode)
    {
        QUrl url = d->trashView->lastSelectedItemUrl();

        if (url.isValid())
        {
            d->rightSideBar->itemChanged(url);
        }
        else
        {
            d->rightSideBar->slotNoCurrentItem();
        }

        return;
    }

    // the list of ItemInfos of currently selected items, currentItem first

    ApplicationSettings::ApplyToEntireGroup applyAll =
        ApplicationSettings::instance()->getGroupingOperateOnAll(MetadataOps);

    const ItemInfoList& list      = selectedInfoList(true, (applyAll == ApplicationSettings::Yes));
    const ItemInfoList& allImages = allInfo(true);

    if (list.isEmpty())
    {
        d->stackedView->setPreviewItem();

        Q_EMIT signalImageSelected(list, allImages);
        Q_EMIT signalNoCurrentItem();
    }
    else
    {
        d->rightSideBar->itemChanged(list, allImages);

        ItemInfo previousInfo;
        ItemInfo nextInfo;

        if (viewMode() == StackedView::TableViewMode)
        {
            previousInfo = d->tableView->previousInfo();
            nextInfo     = d->tableView->nextInfo();
        }
        else
        {
            previousInfo = d->iconView->previousInfo(list.first());
            nextInfo     = d->iconView->nextInfo(list.first());
        }

        if (
            (viewMode() != StackedView::IconViewMode)  &&
            (viewMode() != StackedView::MapWidgetMode) &&
            (viewMode() != StackedView::TableViewMode)
           )
        {
            d->stackedView->setPreviewItem(list.first(), previousInfo, nextInfo);
        }

        Q_EMIT signalImageSelected(list, allImages);
    }
}

void ItemIconView::slotImageWriteMetadata()
{
    const ItemInfoList& selected     = selectedInfoList(MetadataOps);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(selected,
                                                                MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setUseMultiCoreCPU(false);

    if (MetadataHubMngr::isCreated())
    {
        connect (tool, SIGNAL(signalRemovePending(ItemInfo)),
                 MetadataHubMngr::instance(), SLOT(slotRemovePending(ItemInfo)),
                 Qt::QueuedConnection);
    }

    tool->start();
}

void ItemIconView::slotImageReadMetadata()
{
    const ItemInfoList& selected     = selectedInfoList(MetadataOps);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(selected, MetadataSynchronizer::ReadFromFileToDatabase);
    tool->setUseMultiCoreCPU(false);
    tool->start();
}

void ItemIconView::slotRefreshImagePreview()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedView->imagePreviewView()->reload();
    }
}

void ItemIconView::slotImageRename()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->rename();
            break;
        }

        default:
        {
            d->iconView->rename();
            break;
        }
    }
}

void ItemIconView::slotImageDelete()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotDeleteSelected(ItemViewUtilities::DeleteUseTrash);
            break;
        }

        default:
        {

#ifdef HAVE_MEDIAPLAYER

            if (viewMode() == StackedView::MediaPlayerMode)
            {
                d->stackedView->mediaPlayerView()->escapePreview();
            }

#endif // HAVE_MEDIAPLAYER

            d->iconView->deleteSelected(ItemViewUtilities::DeleteUseTrash);
            break;
        }
    }
}

void ItemIconView::slotImageDeletePermanently()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotDeleteSelected(ItemViewUtilities::DeletePermanently);
            break;
        }

        default:
        {

#ifdef HAVE_MEDIAPLAYER

            if (viewMode() == StackedView::MediaPlayerMode)
            {
                d->stackedView->mediaPlayerView()->escapePreview();
            }

#endif // HAVE_MEDIAPLAYER

            d->iconView->deleteSelected(ItemViewUtilities::DeletePermanently);
            break;
        }
    }
}

void ItemIconView::slotImageDeletePermanentlyDirectly()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotDeleteSelectedWithoutConfirmation(ItemViewUtilities::DeletePermanently);
            break;
        }

        default:
        {
            d->iconView->deleteSelectedDirectly(ItemViewUtilities::DeletePermanently);
            break;
        }
    }
}

void ItemIconView::slotImageTrashDirectly()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotDeleteSelectedWithoutConfirmation(ItemViewUtilities::DeleteUseTrash);
            break;
        }

        default:
        {
            d->iconView->deleteSelectedDirectly(ItemViewUtilities::DeleteUseTrash);
            break;
        }
    }
}

void ItemIconView::slotSelectAll()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->selectAll();
            break;
        }

        default:
        {
            d->iconView->selectAll();
            break;
        }
    }
}

void ItemIconView::slotSelectNone()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->clearSelection();
            break;
        }

        default:
        {
            d->iconView->clearSelection();
            break;
        }
    }
}

void ItemIconView::slotSelectInvert()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->invertSelection();
            break;
        }

        default:
        {
            d->iconView->invertSelection();
            break;
        }
    }
}

void ItemIconView::slotSortImages(int sortRole)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSortOrder(sortRole);
    d->iconView->itemFilterModel()->setSortRole((ItemSortSettings::SortRole) sortRole);
    settings->emitSetupChanged();
}

void ItemIconView::slotSortImagesOrder(int order)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSorting(order);
    d->iconView->itemFilterModel()->setSortOrder((ItemSortSettings::SortOrder) order);
    settings->emitSetupChanged();
}

void ItemIconView::slotSeparateImages(int categoryMode)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSeparationMode(categoryMode);
    d->iconView->itemFilterModel()->setCategorizationMode((ItemSortSettings::CategorizationMode) categoryMode);
}

void ItemIconView::slotImageSeparationSortOrder(int order)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSeparationSortOrder(order);
    d->iconView->itemFilterModel()->setCategorizationSortOrder((ItemSortSettings::SortOrder) order);
}

void ItemIconView::slotImagePaste()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotPaste();
            break;
        }

        default:
        {
            d->iconView->paste();
            break;
        }
    }
}

void ItemIconView::slotImageChangeFailed(const QString& message, const QStringList& fileNames)
{
    if (fileNames.isEmpty())
    {
        return;
    }

    DMessageBox::showInformationList(QMessageBox::Critical,
                                     qApp->activeWindow(),
                                     qApp->applicationName(),
                                     message,
                                     fileNames);
}

bool ItemIconView::hasCurrentItem() const
{
    return !currentInfo().isNull();
}

void ItemIconView::slotFocusAndNextImage()
{
    // slot is called on pressing "return" a second time after assigning a tag

    d->stackedView->currentWidget()->setFocus();

    // select next image, since the user is probably done tagging the current image

    slotNextItem();
}

void ItemIconView::slotImageExifOrientation(int orientation)
{
    FileActionMngr::instance()->setExifOrientation(
                selectedInfoList(MetadataOps), orientation);
}

void ItemIconView::imageTransform(MetaEngineRotation::TransformationAction transform)
{
    FileActionMngr::instance()->transform(
                selectedInfoList(MetadataOps), transform);
}

ItemInfo ItemIconView::currentInfo() const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            return d->tableView->currentInfo();
        }

#ifdef HAVE_GEOLOCATION

        case StackedView::MapWidgetMode:
        {
            return d->mapView->currentItemInfo();
        }

#endif // HAVE_GEOLOCATION

        case StackedView::MediaPlayerMode:
        case StackedView::PreviewImageMode:
        case StackedView::IconViewMode:
        {
            // all of these modes use the same selection model and data as the IconViewMode

            return d->iconView->currentInfo();
        }

        default:
        {
            return ItemInfo();
        }
    }
}

ItemInfoList ItemIconView::selectedInfoList(const bool currentFirst,
                                            const bool grouping) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {

            if (currentFirst)
            {
                return d->tableView->selectedItemInfosCurrentFirst(grouping);
            }

            return d->tableView->selectedItemInfos(grouping);
        }

        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::MapWidgetMode:
        case StackedView::IconViewMode:
        {

            // all of these modes use the same selection model and data as the IconViewMode

            if (currentFirst)
            {
                return d->iconView->selectedItemInfosCurrentFirst(grouping);
            }

            return d->iconView->selectedItemInfos(grouping);
        }

        default:
        {
            return ItemInfoList();
        }
    }
}

ItemInfoList ItemIconView::selectedInfoList(const OperationType type,
                                            const bool currentFirst) const
{
    return selectedInfoList(currentFirst, selectedNeedGroupResolving(type));
}

ItemInfoList ItemIconView::allInfo(const bool grouping) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            return d->tableView->allItemInfos(grouping);
        }

        case StackedView::MapWidgetMode:
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::IconViewMode:
        {
            // all of these modes use the same selection model and data as the IconViewMode

            return d->iconView->allItemInfos(grouping);
        }

        default:
        {
            return ItemInfoList();
        }
    }
}

ItemInfoList ItemIconView::allInfo(const OperationType type) const
{
    return allInfo(allNeedGroupResolving(type));
}

int ItemIconView::itemCount() const
{
    return d->iconView->itemModel()->itemCount();
}

QUrl ItemIconView::currentUrl() const
{
    const ItemInfo cInfo = currentInfo();

    return cInfo.fileUrl();
}

void ItemIconView::slotSetCurrentUrlWhenAvailable(const QUrl& url)
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotSetCurrentUrlWhenAvailable(url);
            break;
        }

        default:
        {
            d->iconView->setCurrentUrlWhenAvailable(url);
            break;
        }
    }
}

void ItemIconView::slotSetCurrentWhenAvailable(const qlonglong id)
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotSetCurrentWhenAvailable(id);
            break;
        }

        default:
        {
            d->iconView->setCurrentWhenAvailable(id);
            break;
        }
    }
}

void ItemIconView::slotAwayFromSelection()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            d->tableView->slotAwayFromSelection();
            break;
        }

        default:
        {
            d->iconView->awayFromSelection();
            break;
        }
    }
}

void ItemIconView::slotCopySelectionTo()
{
    d->utilities->copyItemsToExternalFolder(selectedInfoList(false, true));
}

void ItemIconView::slotEmptyMessageTimer()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (albumList.isEmpty())
    {
        return;
    }

   const Album* const album = albumList.first();

    if (!album || album->isRoot() || (album->type() != Album::SEARCH))
    {
        if (d->errorWidget->messageType() == DNotificationWidget::Notification)
        {
            d->errorWidget->animatedHide();
        }

        return;
    }

    if      (!d->iconView->itemModel()->itemCount())
    {
        QString message = i18n("The current query from the database returned no results. "
                               "Change search settings or select an album with content.");

        d->errorWidget->setMessageType(DNotificationWidget::Notification);
        d->errorWidget->setText(message);
        d->errorWidget->animatedShowTemporized(5000);
    }
    else if (d->errorWidget->messageType() == DNotificationWidget::Notification)
    {
        d->errorWidget->animatedHide();
    }
}

} // namespace Digikam
