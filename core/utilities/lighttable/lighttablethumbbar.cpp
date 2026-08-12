/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lighttablethumbbar.h"

// Qt includes

#include <QAction>
#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "applicationsettings.h"
#include "contextmenuhelper.h"
#include "itemfiltermodel.h"
#include "itemdragdrop.h"
#include "lighttableitemlistmodel.h"
#include "fileactionmngr.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN LightTableThumbBar::Private
{
public:

    Private() = default;

public:

    bool                     navigateByPair     = false;

    LightTableItemListModel* imageInfoModel     = nullptr;
    ItemFilterModel*         imageFilterModel   = nullptr;
    ItemDragDropHandler*     dragDropHandler    = nullptr;
};

LightTableThumbBar::LightTableThumbBar(QWidget* const parent)
    : ItemThumbnailBar(parent),
      d               (new Private)
{
    d->imageInfoModel   = new LightTableItemListModel(this);

    // only one is left, only one is right at a time

    d->imageInfoModel->setExclusiveLightTableState(true);

    d->imageFilterModel = new ItemFilterModel(this);
    d->imageFilterModel->setSourceItemModel(d->imageInfoModel);

    d->imageInfoModel->setWatchFlags(d->imageFilterModel->suggestedWatchFlags());
    d->imageInfoModel->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    ApplicationSettings* const settings = ApplicationSettings::instance();

    d->imageFilterModel->setCategorizationMode(ItemSortSettings::NoCategories);
    d->imageFilterModel->setStringTypeNatural(settings->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ItemSortSettings::SortRole)settings->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ItemSortSettings::SortOrder)settings->getImageSorting());
    d->imageFilterModel->setAllGroupsOpen(true); // disable filtering out by group, see bug #308948
    d->imageFilterModel->sort(0);                // an initial sorting is necessary

    d->dragDropHandler = new ItemDragDropHandler(d->imageInfoModel);
    d->dragDropHandler->setReadOnlyDrop(true);
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    setModels(d->imageInfoModel, d->imageFilterModel);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(d->dragDropHandler, SIGNAL(itemInfosDropped(QList<ItemInfo>)),
            this, SIGNAL(signalDroppedItems(QList<ItemInfo>)));

    connect(d->imageInfoModel, SIGNAL(imageInfosAdded(QList<ItemInfo>)),
            this, SIGNAL(signalContentChanged()));

    connect(d->imageInfoModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(signalContentChanged()));

    connect(settings, SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

LightTableThumbBar::~LightTableThumbBar()
{
    delete d;
}

void LightTableThumbBar::setItems(const ItemInfoList& list)
{
    for (const ItemInfo& info : std::as_const(list))
    {
        if (!d->imageInfoModel->hasImage(info))
        {
            d->imageInfoModel->addItemInfo(info);
        }
    }
}

void LightTableThumbBar::clear()
{
    d->imageInfoModel->clearItemInfos();

    Q_EMIT signalContentChanged();
}

void LightTableThumbBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableThumbBar::showContextMenuOnInfo(QContextMenuEvent* e, const ItemInfo& info)
{
    // temporary actions ----------------------------------

    QAction* const leftPanelAction  = new QAction(QIcon::fromTheme(QLatin1String("arrow-left")),    i18n("Show on left panel"),  this);
    QAction* const rightPanelAction = new QAction(QIcon::fromTheme(QLatin1String("arrow-right")),   i18n("Show on right panel"), this);
    QAction* const editAction       = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Edit"),                this);
    QAction* const removeAction     = new QAction(QIcon::fromTheme(QLatin1String("window-close")),  i18n("Remove item"),         this);
    QAction* const clearAllAction   = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),   i18n("Clear all"),           this);

    leftPanelAction->setEnabled(d->navigateByPair  ? false : true);
    rightPanelAction->setEnabled(d->navigateByPair ? false : true);
    clearAllAction->setEnabled(countItems()        ? true  : false);

    // ----------------------------------------------------

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.addAction(leftPanelAction, true);
    cmhelper.addAction(rightPanelAction, true);
    cmhelper.addSeparator();
    cmhelper.addAction(editAction);
    cmhelper.addServicesMenu(QList<QUrl>() << info.fileUrl());
    cmhelper.addSeparator();
    cmhelper.addLabelsAction();
    cmhelper.addSeparator();
    cmhelper.addAction(removeAction);
    cmhelper.addAction(clearAllAction, true);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    const QAction* const choice = cmhelper.exec(e->globalPos());

    if (choice)
    {
        if      (choice == leftPanelAction)
        {
            Q_EMIT signalSetItemOnLeftPanel(info);
        }
        else if (choice == rightPanelAction)
        {
            Q_EMIT signalSetItemOnRightPanel(info);
        }
        else if (choice == editAction)
        {
            Q_EMIT signalEditItem(info);
        }
        else if (choice == removeAction)
        {
            Q_EMIT signalRemoveItem(info);
        }
        else if (choice == clearAllAction)
        {
            Q_EMIT signalClearAll();
        }
    }
}

void LightTableThumbBar::slotColorLabelChanged(const QUrl& url, int color)
{
    assignColorLabel(ItemInfo::fromUrl(url), color);
}

void LightTableThumbBar::slotPickLabelChanged(const QUrl& url, int pick)
{
    assignPickLabel(ItemInfo::fromUrl(url), pick);
}

void LightTableThumbBar::slotAssignPickLabel(int pickId)
{
    assignPickLabel(currentInfo(), pickId);
}

void LightTableThumbBar::slotAssignColorLabel(int colorId)
{
    assignColorLabel(currentInfo(), colorId);
}

void LightTableThumbBar::slotRatingChanged(const QUrl& url, int rating)
{
    assignRating(ItemInfo::fromUrl(url), rating);
}

void LightTableThumbBar::slotAssignRating(int rating)
{
    assignRating(currentInfo(), rating);
}

void LightTableThumbBar::assignPickLabel(const ItemInfo& info, int pickId)
{
    FileActionMngr::instance()->assignPickLabel(info, pickId);
}

void LightTableThumbBar::assignRating(const ItemInfo& info, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    FileActionMngr::instance()->assignRating(info, rating);
}

void LightTableThumbBar::assignColorLabel(const ItemInfo& info, int colorId)
{
    FileActionMngr::instance()->assignColorLabel(info, colorId);
}

void LightTableThumbBar::slotToggleTag(const QUrl& url, int tagID)
{
    toggleTag(ItemInfo::fromUrl(url), tagID);
}

void LightTableThumbBar::toggleTag(int tagID)
{
    toggleTag(currentInfo(), tagID);
}

void LightTableThumbBar::toggleTag(const ItemInfo& info, int tagID)
{
    if (!info.isNull())
    {
        if (!info.tagIds().contains(tagID))
        {
            FileActionMngr::instance()->assignTag(info, tagID);
        }
        else
        {
            FileActionMngr::instance()->removeTag(info, tagID);
        }
    }
}

void LightTableThumbBar::setOnLeftPanel(const ItemInfo& info)
{
    QModelIndex index = d->imageInfoModel->indexForItemInfo(info);

    // model has exclusiveLightTableState, so any previous index will be reset

    d->imageInfoModel->setData(index, true, ItemModel::LTLeftPanelRole);
    viewport()->update();
}

void LightTableThumbBar::setOnRightPanel(const ItemInfo& info)
{
    QModelIndex index = d->imageInfoModel->indexForItemInfo(info);

    // model has exclusiveLightTableState, so any previous index will be reset

    d->imageInfoModel->setData(index, true, ItemModel::LTRightPanelRole);
    viewport()->update();
}

bool LightTableThumbBar::isOnLeftPanel(const ItemInfo& info) const
{
    return d->imageInfoModel->indexForItemInfo(info).data(ItemModel::LTLeftPanelRole).toBool();
}

bool LightTableThumbBar::isOnRightPanel(const ItemInfo& info) const
{
    return d->imageInfoModel->indexForItemInfo(info).data(ItemModel::LTRightPanelRole).toBool();
}

QModelIndex LightTableThumbBar::findItemByInfo(const ItemInfo& info) const
{
    if (!info.isNull())
    {
        return d->imageInfoModel->indexForItemInfo(info);
    }

    return QModelIndex();
}

ItemInfo LightTableThumbBar::findItemByIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return d->imageInfoModel->imageInfo(index);
    }

    return ItemInfo();
}

void LightTableThumbBar::removeItemByInfo(const ItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    d->imageInfoModel->removeItemInfo(info);
}

int LightTableThumbBar::countItems() const
{
    return d->imageInfoModel->rowCount();
}

void LightTableThumbBar::paintEvent(QPaintEvent* e)
{
    if (!countItems())
    {
        QPainter p(viewport());
        p.setPen(QPen(qApp->palette().color(QPalette::Text)));
        p.drawText(0, 0, width(), height(),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   i18n("Drag and drop images here"));
        p.end();

        return;
    }

    ItemThumbnailBar::paintEvent(e);
}

void LightTableThumbBar::slotSetupChanged()
{
    const ApplicationSettings* const settings = ApplicationSettings::instance();

    d->imageFilterModel->setStringTypeNatural(settings->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ItemSortSettings::SortRole)settings->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ItemSortSettings::SortOrder)settings->getImageSorting());
}

ItemFilterModel* LightTableThumbBar::itemFilterModel() const
{
    return d->imageFilterModel;
}

} // namespace Digikam

#include "moc_lighttablethumbbar.cpp"
