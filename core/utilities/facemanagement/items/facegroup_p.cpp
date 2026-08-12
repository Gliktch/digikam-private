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

FaceGroup::Private::Private(FaceGroup* const qq)
    : q(qq)
{
    newEditPipeline = FacePipelineEdit::instance();
}

QList<QGraphicsItem*> FaceGroup::Private::hotItems(const QPointF& scenePos)
{
    if (!q->hasVisibleItems())
    {
        return QList<QGraphicsItem*>();
    }

    const int distance               = 15;
    QRectF hotSceneRect              = QRectF(scenePos, QSize(0, 0)).adjusted(-distance, -distance, distance, distance);
    QList<QGraphicsItem*> closeItems = view->scene()->items(hotSceneRect, Qt::IntersectsItemBoundingRect);

    closeItems.removeOne(view->previewItem());

    return closeItems;
}

void FaceGroup::Private::applyVisible()
{
    if      (state == NoFaces)
    {
        // If not yet loaded, load. load() will transitionToVisible after loading.

        q->load();
    }
    else if (state == FacesLoaded)
    {
        // Show existing faces, if we have an image.

        if (view->previewItem()->isLoaded())
        {
            visibilityController->show();
        }
    }
}

void FaceGroup::Private::clear(FaceGroupState fs)
{
    q->slotCancelAddItem();
    visibilityController->clear();

    for (FaceItem* const item : std::as_const(items))
    {
        delete item;
    }

    items.clear();
    state = fs;
}

FaceItem* FaceGroup::Private::createItem(const FaceTagsIface& face)
{
    FaceItem* const item = new FaceItem(view->previewItem());
    item->setFace(face);

    QRect faceRect       = face.region().toRect();

    if (!exifRotate)
    {
        TagRegion::reverseToOrientation(faceRect,
                                        info.orientation(),
                                        info.dimensions());
    }

    item->setOriginalRect(faceRect);
    item->setVisible(false);

    return item;
}

FaceItem* FaceGroup::Private::addItem(const FaceTagsIface& face)
{
    FaceItem* const item                 = createItem(face);

    // For identification, use index in our list.

    QList<QVariant> identifier(face.toVariant().toList());
    identifier << items.size();

    AssignNameWidget* const assignWidget = createAssignNameWidget(face, identifier);
    item->setHudWidget(assignWidget);
/*
        new StyleSheetDebugger(assignWidget);
*/
    visibilityController->addItem(item);

    items << item;

    return item;
}

void FaceGroup::Private::checkModels()
{
    if (!tagModel)
    {
        tagModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, q);
    }

    if (!filterModel)
    {
        filterModel = new CheckableAlbumFilterModel(q);
    }

    if (!filteredModel)
    {
        filteredModel = new TagPropertiesFilterModel(q);
    }
}

AssignNameWidget::Mode FaceGroup::Private::assignWidgetMode(FaceTagsIface::Type type)
{
    switch (type)
    {
        case FaceTagsIface::UnknownName:
        case FaceTagsIface::UnconfirmedName:
        {
            return AssignNameWidget::UnconfirmedEditMode;
        }

        case FaceTagsIface::Type::IgnoredName:
        {
            return AssignNameWidget::IgnoredMode;
        }

        case FaceTagsIface::ConfirmedName:
        {
            return AssignNameWidget::ConfirmedMode;
        }

        default:
        {
            return AssignNameWidget::InvalidMode;
        }
    }
}

AssignNameWidget* FaceGroup::Private::createAssignNameWidget(const FaceTagsIface& face, const QVariant& identifier)
{
    AssignNameWidget* const assignWidget = new AssignNameWidget(view);
    assignWidget->setMode(assignWidgetMode(face.type()));
    assignWidget->setTagEntryWidgetMode(AssignNameWidget::AddTagsComboBoxMode);
    assignWidget->setVisualStyle(AssignNameWidget::TranslucentDarkRound);
    assignWidget->setLayoutMode(AssignNameWidget::TwoLines);
    assignWidget->setUserData(info, identifier);
    checkModels();
    assignWidget->setAlbumModels(tagModel, filteredModel, filterModel);
    assignWidget->setParentTag(AlbumManager::instance()->findTAlbum(FaceTags::personParentTag()));

    q->connect(assignWidget, SIGNAL(assigned(TaggingAction,ItemInfo,QVariant)),
               q, SLOT(slotAssigned(TaggingAction,ItemInfo,QVariant)));

    q->connect(assignWidget, SIGNAL(rejected(ItemInfo,QVariant)),
               q, SLOT(slotRejected(ItemInfo,QVariant)));

    q->connect(assignWidget, SIGNAL(ignored(ItemInfo,QVariant)),
               q, SLOT(slotIgnored(ItemInfo,QVariant)));

    q->connect(assignWidget, SIGNAL(ignoredClicked(ItemInfo,QVariant)),
               q, SLOT(slotIgnoredClicked(ItemInfo,QVariant)));

    q->connect(assignWidget, SIGNAL(labelClicked(ItemInfo,QVariant)),
               q, SLOT(slotLabelClicked(ItemInfo,QVariant)));

    return assignWidget;
}

} // namespace Digikam
