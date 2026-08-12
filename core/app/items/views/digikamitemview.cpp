/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt model-view for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamitemview_p.h"

// Qt includes

#include <QApplication>
#include <QPointer>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "coredb.h"
#include "coredboperationgroup.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "assignnameoverlay.h"
#include "contextmenuhelper.h"
#include "coredbaccess.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "digikamitemdelegate.h"
#include "itemfacedelegate.h"
#include "dio.h"
#include "groupindicatoroverlay.h"
#include "itemalbumfiltermodel.h"
#include "itemalbummodel.h"
#include "itemdragdrop.h"
#include "itemratingoverlay.h"
#include "itemfullscreenoverlay.h"
#include "itemcoordinatesoverlay.h"
#include "tagslineeditoverlay.h"
#include "imagewindow.h"
#include "fileactionmngr.h"
#include "fileactionprogress.h"
#include "thumbnailloadthread.h"
#include "tagregion.h"
#include "addtagslineedit.h"
#include "facerejectionoverlay.h"
#include "facetagsiface.h"
#include "faceutils.h"
#include "identityprovider.h"
#include "facesengine.h"

namespace Digikam
{

DigikamItemView::DigikamItemView(QWidget* const parent)
    : ItemCategorizedView(parent),
      d                  (new Private(this))
{
    installDefaultModels();

    d->newEditPipeline = FacePipelineEdit::instance();
    d->newEditPipeline->start();

    connect(d->newEditPipeline, SIGNAL(scheduled()),
            this, SLOT(slotInitProgressIndicator()));

    d->normalDelegate = new DigikamItemDelegate(this);
    d->faceDelegate   = new ItemFaceDelegate(this);

    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    ApplicationSettings* const settings = ApplicationSettings::instance();

    itemFilterModel()->setCategorizationMode(ItemSortSettings::CategoryByAlbum);

    itemAlbumModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    // Virtual method: use Dynamic binding.

    this->setThumbnailSize(ThumbnailSize(settings->getDefaultIconSize()));

    itemAlbumModel()->setPreloadThumbnails(true);

    itemModel()->setDragDropHandler(new ItemDragDropHandler(itemModel()));

    setAutoScroll(true);
    setDragEnabled(true);
    //setAcceptDrops(true);
    setDropIndicatorShown(false);
    setDragDropMode(QAbstractItemView::InternalMove);

    setToolTipEnabled(settings->showToolTipsIsValid());
    itemFilterModel()->setStringTypeNatural(settings->isStringTypeNatural());
    itemFilterModel()->setSortRole((ItemSortSettings::SortRole)settings->getImageSortOrder());
    itemFilterModel()->setSortOrder((ItemSortSettings::SortOrder)settings->getImageSorting());
    itemFilterModel()->setCategorizationMode((ItemSortSettings::CategorizationMode)settings->getImageSeparationMode());
    itemFilterModel()->setCategorizationSortOrder((ItemSortSettings::SortOrder) settings->getImageSeparationSortOrder());

    // --- Plug icon-view overlays 

    // Selection overlay

    addSelectionOverlay(d->normalDelegate);
    addSelectionOverlay(d->faceDelegate);

    // Rotation overlay

    d->rotateLeftOverlay  = ItemRotateOverlay::left(this);
    d->rotateRightOverlay = ItemRotateOverlay::right(this);
    d->fullscreenOverlay  = ItemFullScreenOverlay::instance(this);
    d->updateOverlays();

    // Rating overlay

    ItemRatingOverlay* const ratingOverlay = new ItemRatingOverlay(this);
    addOverlay(ratingOverlay);

    // Face overlay
    // NOTE: order to plug this overlay is important, else rejection cannot be suitable (see bug #324759).

    addAssignNameOverlay(d->faceDelegate);
    addRejectionOverlay(d->faceDelegate);

    // Group overlay

    GroupIndicatorOverlay* const groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);

    // Geolocation overlay

    ItemCoordinatesOverlay* const geoOverlay = new ItemCoordinatesOverlay(this);
    addOverlay(geoOverlay);

    connect(geoOverlay, SIGNAL(signalOpenGeolocationMap(QModelIndex)),
            this, SLOT(slotOpenGeolocationMap(QModelIndex)));

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            this, SLOT(groupIndicatorClicked(QModelIndex)));

    connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    // ---

    d->utilities = new ItemViewUtilities(this);

    connect(itemModel()->dragDropHandler(), SIGNAL(assignTags(QList<ItemInfo>,QList<int>)),
            FileActionMngr::instance(), SLOT(assignTags(QList<ItemInfo>,QList<int>)));

    connect(itemModel()->dragDropHandler(), SIGNAL(addToGroup(ItemInfo,QList<ItemInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(ItemInfo,QList<ItemInfo>)));

    connect(itemModel()->dragDropHandler(), SIGNAL(dragDropSort(ItemInfo,QList<ItemInfo>)),
            this, SLOT(dragDropSort(ItemInfo,QList<ItemInfo>)));

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(QUrl)),
            this, SLOT(setCurrentUrlWhenAvailable(QUrl)));

    // --- NOTE: use dynamic binding as slotSetupChanged() is a virtual method which can be re-implemented in derived classes.

    connect(settings, &ApplicationSettings::setupChanged,
            this, &DigikamItemView::slotSetupChanged);

    this->slotSetupChanged();
}

DigikamItemView::~DigikamItemView()
{
    d->newEditPipeline->cancel();

    while (!d->newEditPipeline->hasFinished())
    {
        QThread::msleep(10);
    }

    delete d;
}

ItemViewUtilities* DigikamItemView::utilities() const
{
    return d->utilities;
}

void DigikamItemView::setThumbnailSize(const ThumbnailSize& size)
{
    itemThumbnailModel()->setPreloadThumbnailSize(size);
    ItemCategorizedView::setThumbnailSize(size);
}

ItemInfoList DigikamItemView::allItemInfos(bool grouping) const
{
    if (grouping)
    {
        return resolveGrouping(ItemCategorizedView::allItemInfos());
    }

    return ItemCategorizedView::allItemInfos();
}

ItemInfoList DigikamItemView::selectedItemInfos(bool grouping) const
{
    if (grouping)
    {
        return resolveGrouping(ItemCategorizedView::selectedItemInfos());
    }

    return ItemCategorizedView::selectedItemInfos();
}

ItemInfoList DigikamItemView::selectedItemInfosCurrentFirst(bool grouping) const
{
    if (grouping)
    {
        return resolveGrouping(ItemCategorizedView::selectedItemInfosCurrentFirst());
    }

    return ItemCategorizedView::selectedItemInfosCurrentFirst();
}

void DigikamItemView::dragDropSort(const ItemInfo& pick, const QList<ItemInfo>& infos)
{
    if (pick.isNull() || infos.isEmpty())
    {
        return;
    }

    ItemInfoList infoList = allItemInfos(false);
    qlonglong counter     = pick.manualOrder();
    bool order            = (ApplicationSettings::instance()->
                               getImageSorting() == Qt::AscendingOrder);
    bool found            = false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    CoreDbOperationGroup group;
    group.setMaximumTime(200);

    for (ItemInfo info : std::as_const(infoList))
    {
        if      (!found && info.id() == pick.id())
        {
            for (ItemInfo dropInfo : std::as_const(infos))
            {
                dropInfo.setManualOrder(counter);
                counter += (order ? 1 : -1);
            }

            info.setManualOrder(counter);
            found = true;
        }
        else if (found && !infos.contains(info))
        {
            if (
                ( order && info.manualOrder() > counter) ||
                (!order && info.manualOrder() < counter)
               )
            {
                break;
            }

            counter += (order ? 100 : -100);
            info.setManualOrder(counter);
        }

        group.allowLift();
    }

    QApplication::restoreOverrideCursor();

    itemFilterModel()->invalidate();
 }

bool DigikamItemView::allNeedGroupResolving(const OperationType type) const
{
    return needGroupResolving(type, allItemInfos());
}

bool DigikamItemView::selectedNeedGroupResolving(const OperationType type) const
{
    return needGroupResolving(type, selectedItemInfos());
}

int DigikamItemView::fitToWidthIcons()
{
    return itemDelegate()->calculatethumbSizeToFit(viewport()->size().width());
}

void DigikamItemView::slotSetupChanged()
{
    itemFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    setInitialSelectedItem(ApplicationSettings::instance()->getSelectFirstAlbumItem());
    setToolTipEnabled(ApplicationSettings::instance()->showToolTipsIsValid());
    setFont(ApplicationSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ItemCategorizedView::slotSetupChanged();
}

bool DigikamItemView::hasHiddenGroupedImages(const ItemInfo& info) const
{
    return (
            info.hasGroupedImages()               &&
            !itemFilterModel()->isAllGroupsOpen() &&
            !itemFilterModel()->isGroupOpen(info.id())
           );
}

ItemInfoList DigikamItemView::imageInfos(const QList<QModelIndex>& indexes,
                                         OperationType type) const
{
    ItemInfoList infos = ItemCategorizedView::imageInfos(indexes);

    if (needGroupResolving(type, infos))
    {
        return resolveGrouping(infos);
    }

    return infos;
}

bool DigikamItemView::isPeopleViewMode() const
{
    return (d->sidebarViewMode == PeopleView);
}

void DigikamItemView::setSidebarViewMode(int mode)
{
    d->sidebarViewMode     = mode;

    // Groups open view and separation mode from settings default.

    QString tagListing;
    ItemDelegate* delegate = d->normalDelegate;
    bool showGroupsOpen    = ApplicationSettings::instance()->getAllGroupsOpen();
    int separationMode     = ApplicationSettings::instance()->getImageSeparationMode();

    if      (mode == SearchView)
    {
        showGroupsOpen = true;
    }
    else if (mode == FuzzySView)
    {
        showGroupsOpen = true;
    }
    else if (mode == PeopleView)
    {
        // See ItemLister, which creates a search the
        // implements listing tag in the ioslave

        tagListing     = QLatin1String("faces");
        delegate       = d->faceDelegate;

        // Grouping is not very much compatible with faces.

        showGroupsOpen = true;

        // By default, Face View is categorized by Faces.

        separationMode = ItemSortSettings::CategoryByFaces;
    }

    itemAlbumModel()->setSpecialTagListing(tagListing);
    setItemDelegate(delegate);

    itemFilterModel()->setAllGroupsOpen(showGroupsOpen);
    itemFilterModel()->setCategorizationMode((ItemSortSettings::CategorizationMode)separationMode);

    Q_EMIT signalSeparationModeChanged(separationMode);
}

void DigikamItemView::addRejectionOverlay(ItemDelegate* delegate)
{
    FaceRejectionOverlay* const rejectionOverlay = new FaceRejectionOverlay(this);

    connect(rejectionOverlay, SIGNAL(rejectFaces(QList<QModelIndex>)),
            this, SLOT(removeFaces(QList<QModelIndex>)));

    addOverlay(rejectionOverlay, delegate);
}

/*
void DigikamItemView::addTagEditOverlay(ItemDelegate* delegate)
{
    TagsLineEditOverlay* tagOverlay = new TagsLineEditOverlay(this);

    connect(tagOverlay, SIGNAL(signalTagEditedByKeyword(QModelIndex,QString)),
            this, SLOT(assignTag(QModelIndex,QString)));

    addOverlay(tagOverlay, delegate);
}
*/

void DigikamItemView::addAssignNameOverlay(ItemDelegate* delegate)
{
    AssignNameOverlay* const nameOverlay = new AssignNameOverlay(this);
    addOverlay(nameOverlay, delegate);

    connect(nameOverlay, SIGNAL(confirmFaces(QList<QModelIndex>,int)),
            this, SLOT(confirmFaces(QList<QModelIndex>,int)));

    connect(nameOverlay, SIGNAL(removeFaces(QList<QModelIndex>)),
            this, SLOT(rejectFaces(QList<QModelIndex>)));

    connect(nameOverlay, SIGNAL(unknownFaces(QList<QModelIndex>)),
            this, SLOT(unknownFaces(QList<QModelIndex>)));

    connect(nameOverlay, SIGNAL(ignoreFaces(QList<QModelIndex>)),
            this, SLOT(ignoreFaces(QList<QModelIndex>)));
}

void DigikamItemView::confirmFaces(const QList<QModelIndex>& indexes, int tagId)
{
    /**
     * You aren't allowed to "confirm" a person as
     * Ignored. Marking as Ignored is treated as a
     * changeTag() operation.
     */
    if (FaceTags::isTheIgnoredPerson(tagId))
    {
        rejectFaces(indexes);
        return;
    }

    QModelIndex          next;
    QList<ItemInfo>      infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex>   sourceIndexes;

    // fast-remove in the "unknown person" view

    const Album* const album  = currentAlbum();
    bool needFastRemove       = false;

    if (album)
    {
        needFastRemove = (isPeopleViewMode() && (tagId != album->id()));
    }

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            faces << d->faceDelegate->face(index);
            infos << ItemModel::retrieveItemInfo(index);

            if (needFastRemove)
            {
                sourceIndexes << itemSortFilterModel()->mapToSourceItemModel(index);
            }
        }
    }

    if (!indexes.isEmpty() && indexes.constLast().isValid())
    {
        next = itemSortFilterModel()->index(indexes.constLast().row(), 0, QModelIndex());
    }

    if (infos.size() > 1)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "DigikamItemView::confirmFaces(): INFO: more than 1 face confirmed";
    }

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        d->newEditPipeline->confirmFace(infos[i], faces[i], TagRegion(), tagId, i == (infos.size() - 1));
    }

    itemAlbumModel()->removeIndexes(sourceIndexes);

    if (next.isValid())
    {
        setCurrentIndex(next);
    }

    clearSelection();
}

void DigikamItemView::removeFaces(const QList<QModelIndex>& indexes)
{
    QModelIndex     next;
    QList<ItemInfo> infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex> sourceIndexes;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            faces         << d->faceDelegate->face(index);
            infos         << ItemModel::retrieveItemInfo(index);
            sourceIndexes << itemSortFilterModel()->mapToSourceItemModel(index);
        }
    }

    if (!indexes.isEmpty() && indexes.constLast().isValid())
    {
        next = itemSortFilterModel()->index(indexes.constLast().row(), 0, QModelIndex());
    }

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        d->newEditPipeline->removeFace(infos[i], faces[i]);
    }
}

void DigikamItemView::unknownFaces(const QList<QModelIndex>& indexes)
{
    QModelIndex     next;
    QList<ItemInfo> infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex> sourceIndexes;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            faces         << d->faceDelegate->face(index);
            infos         << ItemModel::retrieveItemInfo(index);
            sourceIndexes << itemSortFilterModel()->mapToSourceItemModel(index);
        }
    }

    if (!indexes.isEmpty() && indexes.constLast().isValid())
    {
        next = itemSortFilterModel()->index(indexes.constLast().row(), 0, QModelIndex());
    }

    itemAlbumModel()->removeIndexes(sourceIndexes);

    if (next.isValid())
    {
        setCurrentIndex(next);
    }

    clearSelection();

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        d->newEditPipeline->editTag(infos[i], faces[i],
                                    FaceTags::unknownPersonTagId());
    }

    itemAlbumModel()->removeIndexes(sourceIndexes);

    if (next.isValid())
    {
        setCurrentIndex(next);
    }

    clearSelection();
}

void DigikamItemView::rejectFaces(const QList<QModelIndex>& indexes)
{
    QModelIndex     next;
    QList<ItemInfo> infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex> sourceIndexes;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            faces         << d->faceDelegate->face(index);
            infos         << ItemModel::retrieveItemInfo(index);
            sourceIndexes << itemSortFilterModel()->mapToSourceItemModel(index);
        }
    }

    if (!indexes.isEmpty() && indexes.constLast().isValid())
    {
        next = itemSortFilterModel()->index(indexes.constLast().row(), 0, QModelIndex());
    }

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        if      (FaceTags::isTheUnknownPerson(faces[i].tagId()))
        {
            // Reject signal was sent from an Unknown Face. Mark as Ignored.

            d->newEditPipeline->editTag(infos[i], faces[i], FaceTags::ignoredPersonTagId());
        }
        else if (FaceTags::isTheIgnoredPerson(faces[i].tagId()))
        {
            // Reject signal was sent from an Ignored Face. Remove face.

            d->newEditPipeline->removeFace(infos[i], faces[i]);
        }
        else
        {
            // Reject face suggestion. Mark as Unknown.

            d->newEditPipeline->editTag(infos[i], faces[i], FaceTags::unknownPersonTagId());
        }
    }

    itemAlbumModel()->removeIndexes(sourceIndexes);

    if (next.isValid())
    {
        setCurrentIndex(next);
    }

    clearSelection();
}

void DigikamItemView::ignoreFaces(const QList<QModelIndex>& indexes)
{
    QModelIndex     next;
    QList<ItemInfo> infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex> sourceIndexes;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            faces         << d->faceDelegate->face(index);
            infos         << ItemModel::retrieveItemInfo(index);
            sourceIndexes << itemSortFilterModel()->mapToSourceItemModel(index);
        }
    }

    if (!indexes.isEmpty() && indexes.constLast().isValid())
    {
        next = itemSortFilterModel()->index(indexes.constLast().row(), 0, QModelIndex());
    }

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        d->newEditPipeline->editTag(infos[i], faces[i],
                                    FaceTags::ignoredPersonTagId());
    }

    itemAlbumModel()->removeIndexes(sourceIndexes);

    if (next.isValid())
    {
        setCurrentIndex(next);
    }

    clearSelection();
}

QList<int> DigikamItemView::getFaceIds(const QList<QModelIndex>& indexes) const
{
    QList<int> ids;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            ids << d->faceDelegate->face(index).tagId();
        }
    }

    return ids;
}

void DigikamItemView::activated(const ItemInfo& info, Qt::KeyboardModifiers modifiers)
{
    if (info.isNull())
    {
        return;
    }

    if (modifiers != Qt::AltModifier)
    {
        int leftClickAction = ApplicationSettings::instance()->getItemLeftClickAction();

        if      (leftClickAction == ApplicationSettings::ShowPreview)
        {
            Q_EMIT previewRequested(info);
        }
        else if (leftClickAction == ApplicationSettings::StartEditor)
        {
            openFile(info);
        }
        else if (leftClickAction == ApplicationSettings::ShowOnTable)
        {
            d->utilities->insertToLightTable(allItemInfos(), info, false);
        }
        else
        {
            d->utilities->openInfosWithDefaultApplication(QList<ItemInfo>() << info);
        }
    }
    else
    {
        d->utilities->openInfosWithDefaultApplication(QList<ItemInfo>() << info);
    }
}

void DigikamItemView::showContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info)
{
    Q_EMIT signalShowContextMenuOnInfo(event, info, QList<QAction*>(), itemFilterModel());
}

void DigikamItemView::showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event)
{
    Q_UNUSED(index);

    Q_EMIT signalShowGroupContextMenu(event, selectedItemInfosCurrentFirst(), itemFilterModel());
}

void DigikamItemView::showContextMenu(QContextMenuEvent* event)
{
    Q_EMIT signalShowContextMenu(event);
}

void DigikamItemView::openFile(const ItemInfo& info)
{
    d->utilities->openInfos(info, allItemInfos(), currentAlbum());
}

void DigikamItemView::deleteSelected(const ItemViewUtilities::DeleteMode deleteMode)
{
    ItemInfoList imageInfoList = selectedItemInfos(true);

    if (d->utilities->deleteImages(imageInfoList, deleteMode))
    {
        awayFromSelection();
    }
}

void DigikamItemView::deleteSelectedDirectly(const ItemViewUtilities::DeleteMode deleteMode)
{
    ItemInfoList imageInfoList = selectedItemInfos(true);

    d->utilities->deleteImagesDirectly(imageInfoList, deleteMode);
    awayFromSelection();
}

void DigikamItemView::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    ItemInfoList infos = imageInfos(indexes, MetadataOps);
    FileActionMngr::instance()->assignRating(infos, rating);
}

void DigikamItemView::slotOpenGeolocationMap(const QModelIndex& index)
{
    ItemInfo info              = itemFilterModel()->imageInfo(index);
    ItemInfoList imageInfoList = selectedItemInfos(true);

    if (!imageInfoList.contains(info))
    {
        setCurrentIndex(index);
    }

    Q_EMIT signalOpenGeolocationMap();
}

void DigikamItemView::groupIndicatorClicked(const QModelIndex& index)
{
    ItemInfo info = itemFilterModel()->imageInfo(index);

    if (info.isNull())
    {
        return;
    }

    setCurrentIndex(index);
    itemFilterModel()->toggleGroupOpen(info.id());
    itemAlbumModel()->ensureHasGroupedImages(info);
}

void DigikamItemView::rename()
{
    ItemInfoList infos = selectedItemInfos();

    if (needGroupResolving(RenameOps, infos))
    {
        infos = resolveGrouping(infos);
    }

    QList<QUrl> urls = infos.toImageUrlList();
    bool loop        = false;
    NewNamesList newNamesList;

    do
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Selected URLs to rename: " << urls;

        QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
        dlg->slotAddImages(urls);

        if (dialogExec(dlg) != QDialog::Accepted)
        {
            delete dlg;
            break;
        }

        if (!loop)
        {
            QUrl nextUrl = nextInOrder(infos.last(), 1).fileUrl();
            setCurrentUrl(nextUrl);
            loop = true;
        }

        newNamesList = dlg->newNames();
        delete dlg;

        setFocus();
        qApp->processEvents();

        if (!newNamesList.isEmpty())
        {
            QPointer<AdvancedRenameProcessDialog> dlg2 = new AdvancedRenameProcessDialog(newNamesList, this);
            (void)dlg2->exec();

            itemFilterModel()->invalidate();
            urls = dlg2->failedUrls();
            delete dlg2;
        }
    }
    while (!urls.isEmpty() && !newNamesList.isEmpty());
}

void DigikamItemView::slotRotateLeft(const QList<QModelIndex>& indexes)
{
    ItemInfoList infos = imageInfos(indexes, MetadataOps);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::Rotate270);
}

void DigikamItemView::slotRotateRight(const QList<QModelIndex>& indexes)
{
    ItemInfoList infos = imageInfos(indexes, MetadataOps);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::Rotate90);
}

void DigikamItemView::slotRotateAuto(const QList<QModelIndex>& indexes)
{
    ItemInfoList infos = imageInfos(indexes, MetadataOps);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::RotateAuto);
}

void DigikamItemView::slotFullscreen(const QList<QModelIndex>& indexes)
{
   QList<ItemInfo> infos = imageInfos(indexes, SlideshowOps);

   if (infos.isEmpty())
   {
        return;
   }

   // Just fullscreen the first.

   const ItemInfo& info = infos.at(0);

   QList<DPluginAction*> actions = DPluginLoader::instance()->
                                   pluginActions(QLatin1String("org.kde.digikam.plugin.generic.SlideShow"),
                                   DigikamApp::instance());

   if (actions.isEmpty())
   {
       return;
   }

   // Trigger SlideShow manual

   actions[0]->setData(info.fileUrl());
   actions[0]->trigger();
}

void DigikamItemView::slotInitProgressIndicator()
{
    if (!ProgressManager::instance()->findItembyId(QLatin1String("FaceActionProgress")))
    {
        FileActionProgress* const item = new FileActionProgress(QLatin1String("FaceActionProgress"));

        connect(d->newEditPipeline, SIGNAL(started(QString)),
                item, SLOT(slotProgressStatus(QString)));

        connect(d->newEditPipeline, SIGNAL(progressValueChanged(float)),
                item, SLOT(slotProgressValue(float)));

        connect(d->newEditPipeline, SIGNAL(finished()),
                item, SLOT(slotCompleted()));
    }
}

void DigikamItemView::scrollTo(const QModelIndex& index, ScrollHint hint)
{
    // We do not want to change the view, when in the "Thumbnails" view in "People"
    // See bugs 444692, 440232, ...

    bool runningFaceAction = (
                              ProgressManager::instance()->findItembyId(FacesEngine::faceScanTaskToString(FaceScanSettings::FaceScanSource::FaceScanWidget)) ||
                              ProgressManager::instance()->findItembyId(FacesEngine::faceScanTaskToString(FaceScanSettings::FaceScanSource::ItemIconView)) ||
                              ProgressManager::instance()->findItembyId(FacesEngine::faceScanTaskToString(FaceScanSettings::FaceScanSource::MaintenanceTool)) ||
                              ProgressManager::instance()->findItembyId(FacesEngine::faceScanTaskToString(FaceScanSettings::FaceScanSource::BackgroundRecognition)) ||
                              ProgressManager::instance()->findItembyId(FacesEngine::faceScanTaskToString(FaceScanSettings::FaceScanSource::BQM))
                             );

    if ((viewMode() == QListView::IconMode) && isPeopleViewMode() && runningFaceAction)
    {
        return;
    }

    ItemCategorizedView::scrollTo(index, hint);
}

} // namespace Digikam

#include "moc_digikamitemview.cpp"
