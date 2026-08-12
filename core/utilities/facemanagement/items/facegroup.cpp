/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Managing of face tag region items on a GraphicsDImgView
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facegroup_p.h"

namespace Digikam
{

FaceGroup::FaceGroup(GraphicsDImgView* const view)
    : QObject(view),
      d      (new Private(this))
{
    d->view                 = view;
    d->visibilityController = new ItemVisibilityController(this);
    d->visibilityController->setShallBeShown(false);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsUpdated(int)),
            this, SLOT(slotAlbumsUpdated(int)));

    connect(view->previewItem(), SIGNAL(stateChanged(int)),
            this, SLOT(itemStateChanged(int)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));
}

FaceGroup::~FaceGroup()
{
    delete d;
}

void FaceGroup::itemStateChanged(int itemState)
{
    switch (itemState)
    {
        case DImgPreviewItem::NoImage:
        case DImgPreviewItem::ImageLoadingFailed:
        {
            d->visibilityController->hide();

            break;
        }

        case DImgPreviewItem::ImageLoaded:
        {
            if (d->state == FacesLoaded)
            {
                d->visibilityController->show();
            }

            break;
        }
    }
}

bool FaceGroup::isVisible() const
{
    return d->visibilityController->shallBeShown();
}

bool FaceGroup::hasVisibleItems() const
{
    return d->visibilityController->hasVisibleItems();
}

ItemInfo FaceGroup::info() const
{
    return d->info;
}

QList<RegionFrameItem*> FaceGroup::items() const
{
    QList<RegionFrameItem*> items;

    for (FaceItem* const item : std::as_const(d->items))                  // cppcheck-suppress constVariablePointer
    {
        items << item;
    }

    return items;
}

void FaceGroup::setAutoSuggest(bool doAutoSuggest)
{
    if (d->autoSuggest == doAutoSuggest)
    {
        return;
    }

    d->autoSuggest = doAutoSuggest;
}

bool FaceGroup::autoSuggest() const
{
    return d->autoSuggest;
}

void FaceGroup::setShowOnHover(bool show)
{
    d->showOnHover = show;
}

bool FaceGroup::showOnHover() const
{
    return d->showOnHover;
}

void FaceGroup::setVisible(bool visible)
{
    d->visibilityController->setShallBeShown(visible);
    d->applyVisible();
}

void FaceGroup::setVisibleItem(RegionFrameItem* item)
{
    d->visibilityController->setItemThatShallBeShown(item);
    d->applyVisible();
}

void FaceGroup::setInfo(const ItemInfo& info)
{
    if (d->info != info)
    {
        d->clear(NoFaces);
    }

    d->info = info;

    if (d->visibilityController->shallBeShown())
    {
        load();
    }
}

void FaceGroup::aboutToSetInfo(const ItemInfo& info)
{
    if (d->info == info)
    {
        return;
    }

    applyItemGeometryChanges();
}

static QPointF closestPointOfRect(const QPointF& p, const QRectF& r)
{
    QPointF cp = p;

    if      (p.x() < r.left())
    {
        cp.setX(r.left());
    }
    else if (p.x() > r.right())
    {
        cp.setX(r.right());
    }

    if      (p.y() < r.top())
    {
        cp.setY(r.top());
    }
    else if (p.y() > r.bottom())
    {
        cp.setY(r.bottom());
    }

    return cp;
}

RegionFrameItem* FaceGroup::closestItem(const QPointF& p, qreal* const manhattanLength) const
{
    RegionFrameItem* closestItem = nullptr;
    qreal minDistance            = 0;
    qreal minCenterDistance      = 0;

    for (RegionFrameItem* const item : std::as_const(d->items))
    {
        QRectF r       = item->boundingRect().translated(item->pos());
        qreal distance = (p - closestPointOfRect(p, r)).manhattanLength();

        if (
            !closestItem             ||
            (distance < minDistance) ||
            ((distance == 0) && ((p - r.center()).manhattanLength()) < minCenterDistance)
           )
        {
            closestItem = item;
            minDistance = distance;

            if (distance == 0)
            {
                minCenterDistance = (p - r.center()).manhattanLength();
            }
        }
    }

    if (manhattanLength)
    {
        *manhattanLength = minDistance;
    }

    return closestItem;
}

bool FaceGroup::acceptsMouseClick(const QPointF& scenePos)
{
    return d->hotItems(scenePos).isEmpty();
}

void FaceGroup::itemHoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (d->showOnHover && !isVisible())
    {
        qreal distance;
        RegionFrameItem* const item = closestItem(e->scenePos(), &distance);

        // There's a possible nuisance when the direct mouse way from hovering pos to HUD widget
        // is not part of the condition. Maybe, we should add a exemption for this case.

        if (distance < d->MaxMouseDistance)
        {
            setVisibleItem(item);
        }
        else
        {
            // Get all items close to pos.

            QList<QGraphicsItem*> hItems = d->hotItems(e->scenePos());

            // This will be the one item shown by mouse over.

            QList<QObject*> visible      = d->visibilityController->visibleItems(ItemVisibilityController::ExcludeFadingOut);

            for (QGraphicsItem* const item2 : std::as_const(hItems))
            {
                for (QObject* const parent : std::as_const(visible))
                {
                    if (static_cast<QGraphicsObject*>(parent)->isAncestorOf(item2))
                    {
                        // cppcheck-suppress useStlAlgorithm
                        return;
                    }
                }
            }

            setVisibleItem(nullptr);
        }
    }
}

void FaceGroup::itemHoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
}

void FaceGroup::itemHoverEnterEvent(QGraphicsSceneHoverEvent*)
{
}

void FaceGroup::leaveEvent(QEvent*)
{
    if (d->showOnHover && !isVisible())
    {
        setVisibleItem(nullptr);
    }
}

void FaceGroup::enterEvent(QEvent*)
{
}

bool FaceGroup::hasUnconfirmed()
{
    for (const FaceItem* const item : std::as_const(d->items))
    {
        if (item->face().isUnconfirmedType())
        {
            return true;
        }
    }

    return false;
}

void FaceGroup::load()
{
    if (d->state == FacesLoad)
    {
        return;
    }

    d->state      = FacesLoad;
    d->exifRotate = (
                        MetaEngineSettings::instance()->settings().exifRotate            ||
                        (
                         (d->view->previewItem()->image().detectedFormat() == DImg::RAW) &&
                         !d->view->previewItem()->image().attribute(QLatin1String("fromRawEmbeddedPreview")).toBool()
                        )
                    );

    if (d->info.isNull())
    {
        d->clear(FacesLoaded);

        return;
    }

    const QList<FaceTagsIface> faces = FaceTagsEditor().databaseFaces(d->info.id());

    if (faces.isEmpty())
    {
        d->clear(FacesLoaded);

        return;
    }

    d->clear(FacesLoad);

    for (const FaceTagsIface& face : faces)
    {
        d->addItem(face);
    }

    if (d->view->previewItem()->isLoaded())
    {
        d->visibilityController->show();
    }

    // See bug 408982.

    if (d->visibilityController->hasVisibleItems())
    {
        d->view->setFocus();
    }

    d->state = FacesLoaded;
}

void FaceGroup::rejectAll()
{
    d->newEditPipeline->removeAllFaces(d->info);
    d->clear(NoFaces);
}

void FaceGroup::markAllAsIgnored()
{
    for (FaceItem* const item : std::as_const(d->items))
    {
        if (
            item->face().isUnknownName()   ||
            item->face().isUnconfirmedName()
           )
        {
            FaceTagsIface face = d->newEditPipeline->editTag(d->info, item->face(),
                                                             FaceTags::ignoredPersonTagId());
            item->setFace(face);

            item->switchMode(AssignNameWidget::IgnoredMode);
        }
    }
}

void FaceGroup::slotAlbumsUpdated(int type)
{
    if (type != Album::TAG)
    {
        return;
    }

    if (d->items.isEmpty())
    {
        return;
    }

    load();
}

void FaceGroup::slotAlbumRenamed(Album* album)
{
    if (!album || (album->type() != Album::TAG))
    {
        return;
    }

    for (FaceItem* const item : std::as_const(d->items))
    {
        if (
            !item->face().isNull() &&
            (item->face().tagId() == album->id())
           )
        {
            item->updateCurrentTag();
        }
    }
}

void FaceGroup::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (!changeset.containsImage(d->info.id()))
    {
        return;
    }

    if (changeset.operation() == ImageTagChangeset::PropertiesChanged)
    {
        if (
            changeset.containsTag(FaceTags::unknownPersonTagId())  ||
            changeset.containsTag(FaceTags::unconfirmedPersonTagId())
           )
        {
            load();
        }
    }
}

void FaceGroup::slotAssigned(const TaggingAction& action, const ItemInfo&, const QVariant& faceIdentifier)
{
    QList<QVariant> faceList(faceIdentifier.toList());

    if (faceList.size() != d->MaxFaceListSize)
    {
        return;
    }

    FaceItem* const item = d->items[faceList[4].toInt()];
    FaceTagsIface face   = item->face();
    QRect faceRect       = item->originalRect();

    if (!d->exifRotate)
    {
        TagRegion::adjustToOrientation(faceRect,
                                       d->info.orientation(),
                                       d->info.dimensions());
    }

    TagRegion currentRegion(faceRect);

    if (
        !face.isConfirmedName()          ||
        (face.region() != currentRegion) ||
        action.shallCreateNewTag()       ||
        (
            action.shallAssignTag() &&
            (action.tagId() != face.tagId())
        )
       )
    {
        int tagId = 0;

        if      (action.shallAssignTag())
        {
            tagId = action.tagId();
        }
        else if (action.shallCreateNewTag())
        {
            QStringList faceNames = action.newTagName().split(QLatin1Char('/'),
                                                              Qt::SkipEmptyParts);

            if (!faceNames.isEmpty())
            {
                tagId = action.parentTagId();

                for (const QString& name : std::as_const(faceNames))
                {
                    tagId = FaceTags::getOrCreateTagForPerson(name.trimmed(), tagId);
                }
            }
        }

        if (FaceTags::isTheUnknownPerson(tagId))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Refusing to assign the unknown person to an image";
            return;
        }

        if (!tagId)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to get person tag";
            return;
        }

        face = d->newEditPipeline->confirmFace(d->info, face, currentRegion, tagId, true);
        item->setFace(face);

        item->switchMode(AssignNameWidget::ConfirmedMode);

        QTimer::singleShot(250, this, SLOT(slotFocusRandomFace()));
    }
}

void FaceGroup::slotFocusRandomFace()
{
    for (const FaceItem* const item : std::as_const(d->items))
    {
        FaceTagsIface face                    = item->face();
        const AddTagsComboBox* const comboBox = item->widget()->comboBox();

        if ((comboBox) && (!face.isConfirmedName()))
        {
            comboBox->lineEdit()->setFocus();

            return;
        }
    }
}

void FaceGroup::slotRejected(const ItemInfo&, const QVariant& faceIdentifier)
{
    QList<QVariant> faceList(faceIdentifier.toList());

    if (faceList.size() == d->MaxFaceListSize)
    {
        FaceItem* const item = d->items[faceList[4].toInt()];
        d->newEditPipeline->removeFace(d->info, item->face());

        item->setFace(FaceTagsIface());
        d->visibilityController->hideAndRemoveItem(item);

        QTimer::singleShot(250, this, SLOT(slotFocusRandomFace()));
    }
}

void FaceGroup::slotIgnored(const ItemInfo&, const QVariant& faceIdentifier)
{
    QList<QVariant> faceList(faceIdentifier.toList());

    if (faceList.size() == d->MaxFaceListSize)
    {
        FaceItem* const item = d->items[faceList[4].toInt()];
        QRect faceRect       = item->originalRect();
        FaceTagsIface face(item->face());

        if (!d->exifRotate)
        {
            TagRegion::adjustToOrientation(faceRect,
                                           d->info.orientation(),
                                           d->info.dimensions());
        }

        TagRegion currentRegion(faceRect);

        face = d->newEditPipeline->editRegion(d->info, face,
                                              currentRegion,
                                              FaceTags::ignoredPersonTagId(), false);

        item->setFace(face);
        item->switchMode(AssignNameWidget::IgnoredMode);

        QTimer::singleShot(250, this, SLOT(slotFocusRandomFace()));
    }
}

void FaceGroup::slotLabelClicked(const ItemInfo&, const QVariant& faceIdentifier)
{
    QList<QVariant> faceList(faceIdentifier.toList());

    if (faceList.size() == d->MaxFaceListSize)
    {
        FaceItem* const item = d->items[faceList[4].toInt()];
        item->switchMode(AssignNameWidget::ConfirmedEditMode);
    }
}

void FaceGroup::slotIgnoredClicked(const ItemInfo&, const QVariant& faceIdentifier)
{
    QList<QVariant> faceList(faceIdentifier.toList());

    if (faceList.size() == d->MaxFaceListSize)
    {
        FaceItem* const item = d->items[faceList[4].toInt()];
        item->switchMode(AssignNameWidget::UnconfirmedEditMode);
    }
}

void FaceGroup::startAutoSuggest()
{
    if (!d->autoSuggest)
    {
        return;
    }
}

void FaceGroup::addFace()
{
    if (d->manuallyAddWrapItem)
    {
        return;
    }

    d->manuallyAddWrapItem = new ClickDragReleaseItem(d->view->previewItem());
    d->manuallyAddWrapItem->setFocus();
    d->view->setFocus();

    connect(d->manuallyAddWrapItem, SIGNAL(started(QPointF)),
            this, SLOT(slotAddItemStarted(QPointF)));

    connect(d->manuallyAddWrapItem, SIGNAL(moving(QRectF)),
            this, SLOT(slotAddItemMoving(QRectF)));

    connect(d->manuallyAddWrapItem, SIGNAL(finished(QRectF)),
            this, SLOT(slotAddItemFinished(QRectF)));

    connect(d->manuallyAddWrapItem, SIGNAL(cancelled()),
            this, SLOT(slotCancelAddItem()));
}

void FaceGroup::slotAddItemStarted(const QPointF& pos)
{
    Q_UNUSED(pos);
}

void FaceGroup::slotAddItemMoving(const QRectF& rect)
{
    if (!d->manuallyAddedItem)
    {
        d->manuallyAddedItem = d->createItem(FaceTagsIface());
        d->visibilityController->addItem(d->manuallyAddedItem);
        d->visibilityController->showItem(d->manuallyAddedItem);
    }

    d->manuallyAddedItem->setRectInSceneCoordinatesAdjusted(rect);
}

void FaceGroup::slotAddItemFinished(const QRectF& rect)
{
    if (d->manuallyAddedItem)
    {
        d->manuallyAddedItem->setRectInSceneCoordinatesAdjusted(rect);
        QRect faceRect = d->manuallyAddedItem->originalRect();
        DImg preview(d->view->previewItem()->image().copy());

        if (!d->exifRotate)
        {
            TagRegion::adjustToOrientation(faceRect,
                                           d->info.orientation(),
                                           d->info.dimensions());
            preview.rotateAndFlip(d->info.orientation());
        }

        TagRegion addRegion(faceRect);
        FaceTagsIface face   = d->newEditPipeline->addManually(d->info,
                                                               preview,
                                                               addRegion,
                                                               false);
        FaceItem* const item = d->addItem(face);
        d->visibilityController->setItemDirectlyVisible(item, true);
        item->switchMode(AssignNameWidget::UnconfirmedEditMode);
        d->manuallyAddWrapItem->stackBefore(item);
    }

    slotCancelAddItem();
}

void FaceGroup::slotCancelAddItem()
{
    delete d->manuallyAddedItem;
    d->manuallyAddedItem = nullptr;

    if (d->manuallyAddWrapItem)
    {
        d->view->scene()->removeItem(d->manuallyAddWrapItem);
        d->manuallyAddWrapItem->deleteLater();
        d->manuallyAddWrapItem = nullptr;
    }
}

void FaceGroup::applyItemGeometryChanges()
{
    if (d->items.isEmpty())
    {
        return;
    }

    for (const FaceItem* const item : std::as_const(d->items))
    {
        if (item->face().isNull())
        {
            continue;
        }

        QRect faceRect = item->originalRect();

        if (!d->exifRotate)
        {
            TagRegion::adjustToOrientation(faceRect,
                                           d->info.orientation(),
                                           d->info.dimensions());
        }

        TagRegion currentRegion(faceRect);

        if (item->face().region() != currentRegion)
        {
            d->newEditPipeline->editRegion(d->info,
                                           item->face(),
                                           currentRegion, -1, false);
        }
    }
}

/*
void ItemPreviewView::trainFaces()
{
    QList<Face> trainList;

    for (Face f : std::as_const(d->currentFaces))
    {
        if (f.name() != "" && !d->faceIface->isFaceTrained(getItemInfo().id(), f.toRect(), f.name()))
        {
            trainList += f;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of training faces" << trainList.size();

    if (trainList.size()!=0)
    {
        d->faceIface->trainWithFaces(trainList);
        d->faceIface->markFacesAsTrained(getItemInfo().id(), trainList);
    }
}

void ItemPreviewView::suggestFaces()
{
    // Assign tentative names to the face list.

    QList<Face> recogList;

    for (Face f : std::as_const(d->currentFaces))
    {
        if (!d->faceIface->isFaceRecognized(getItemInfo().id(), f.toRect(), f.name()) && f.name().isEmpty())
        {
            f.setName(d->faceIface->recognizedName(f));
            d->faceIface->markFaceAsRecognized(getItemInfo().id(), f.toRect(), f.name());

            // If the face wasn't recognized (too distant) don't suggest anything.

            if (f.name().isEmpty())
            {
                continue;
            }
            else
            {
                recogList += f;
            }
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of suggestions = " << recogList.size();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of faceitems = " << d->faceitems.size();

    // Now find the relevant face items and suggest faces.

    for (int i = 0 ; i < recogList.size() ; ++i)
    {
        for (int j = 0 ; j < d->faceitems.size() ; ++j)
        {
            if (recogList[i].toRect() == d->faceitems[j]->originalRect())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Suggesting a name " << recogList[i].name();
                d->faceitems[j]->suggest(recogList[i].name());
                break;
            }
        }
    }
}
*/

} // namespace Digikam

#include "moc_facegroup.cpp"
