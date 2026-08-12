/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-22-07
 * Description : Icon view for import tool items
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importiconview_p.h"

// Qt includes

#include <QDesktopServices>
#include <QPointer>
#include <QAction>
#include <QMenu>

// Local includes

#include "importoverlays.h"
#include "importsettings.h"
#include "camitemsortsettings.h"
#include "fileactionmngr.h"
#include "importdelegate.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "itemviewutilities.h"
#include "importcontextmenu.h"
#include "importdragdrop.h"

namespace Digikam
{

ImportIconView::ImportIconView(QWidget* const parent)
    : ImportCategorizedView(parent),
      d                    (new Private(this))
{
    ImportThumbnailModel* const model    = new ImportThumbnailModel(this);
    ImportFilterModel* const filterModel = new ImportFilterModel(this);

    filterModel->setSourceImportModel(model);
    filterModel->sort(0); // an initial sorting is necessary

    setModels(model, filterModel);

    d->normalDelegate              = new ImportNormalDelegate(this);

    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    ImportSettings* const settings = ImportSettings::instance();

    // Virtual method: use Dynamic binding.
    this->setThumbnailSize(ThumbnailSize(settings->getDefaultIconSize()));

    importItemModel()->setDragDropHandler(new ImportDragDropHandler(importItemModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());

    // selection overlay

    addSelectionOverlay(d->normalDelegate);

/*  TODO
    addSelectionOverlay(d->faceDelegate);
*/
    // rotation overlays

    d->rotateLeftOverlay  = ImportRotateOverlay::left(this);
    d->rotateRightOverlay = ImportRotateOverlay::right(this);

    addOverlay(new ImportDownloadOverlay(this));
    addOverlay(new ImportLockOverlay(this));
    addOverlay(new ImportCoordinatesOverlay(this));

    d->updateOverlays();

    // rating overlay

    ImportRatingOverlay* const ratingOverlay = new ImportRatingOverlay(this);
    addOverlay(ratingOverlay);

/*  TODO
    GroupIndicatorOverlay* groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);
*/
    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

/*  TODO

    connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            this, SLOT(groupIndicatorClicked(QModelIndex)));

    connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    connect(importItemModel()->dragDropHandler(), SIGNAL(assignTags(QList<CamItemInfo>,QList<int>)),
            FileActionMngr::instance(), SLOT(assignTags(QList<CamItemInfo>,QList<int>)));

    connect(importItemModel()->dragDropHandler(), SIGNAL(addToGroup(CamItemInfo,QList<CamItemInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(CamItemInfo,QList<CamItemInfo>)));
*/
    // --- NOTE: use dynamic binding as slotSetupChanged() is a virtual method which can be re-implemented in derived classes.

    connect(settings, &ImportSettings::setupChanged,
            this, &ImportIconView::slotSetupChanged);

    this->slotSetupChanged();
}

ImportIconView::~ImportIconView()
{
    delete d;
}

ItemViewUtilities* ImportIconView::utilities() const
{
    return d->utilities;
}

int ImportIconView::fitToWidthIcons()
{
    return importDelegate()->calculatethumbSizeToFit(viewport()->size().width());
}

CamItemInfo ImportIconView::camItemInfo(const QString& folder, const QString& file)
{
    QUrl url = QUrl::fromLocalFile(folder);
    url      = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + file);
    QModelIndex indexForCamItemInfo = importFilterModel()->indexForPath(url.toLocalFile());

    if (indexForCamItemInfo.isValid())
    {
        return importFilterModel()->camItemInfo(indexForCamItemInfo);
    }

    return CamItemInfo();
}

CamItemInfo& ImportIconView::camItemInfoRef(const QString& folder, const QString& file)
{
    QUrl url = QUrl::fromLocalFile(folder);
    url      = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + file);
    QModelIndex indexForCamItemInfo = importFilterModel()->indexForPath(url.toLocalFile());
    QModelIndex mappedIndex         = importFilterModel()->mapToSource(indexForCamItemInfo);

    return importItemModel()->camItemInfoRef(mappedIndex);
}

void ImportIconView::slotSetupChanged()
{
    setToolTipEnabled(ImportSettings::instance()->showToolTipsIsValid());
    setFont(ImportSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ImportCategorizedView::slotSetupChanged();
}

void ImportIconView::rename()
{
    QList<QUrl>  urls                  = selectedUrls();
    NewNamesList newNamesList;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        newNamesList = dlg->newNames();
    }

    delete dlg;

    if (!newNamesList.isEmpty())
    {
        QPointer<AdvancedRenameProcessDialog> dlg2 = new AdvancedRenameProcessDialog(newNamesList);
        dlg2->exec();

        delete dlg2;
    }
}

void ImportIconView::deleteSelected(bool /*permanently*/)
{
    // FIXME: This way of deletion may not working with camera items.
/*
    CamItemInfoList camItemInfoList = selectedCamItemInfos();

    if (d->utilities->deleteImages(camItemInfoList, permanently))
    {
       awayFromSelection();
    }
*/
}

void ImportIconView::deleteSelectedDirectly(bool /*permanently*/)
{
    // FIXME: This way of deletion may not working with camera items.
/*
    CamItemInfoList camItemInfoList = selectedCamItemInfos();

    d->utilities->deleteImagesDirectly(camItemInfoList, permanently);
*/
    awayFromSelection();
}

void ImportIconView::createGroupFromSelection()
{
    // TODO: Implement grouping in import tool.
/*
    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();
    CamItemInfo groupLeader          = selectedInfos.takeFirst();
    FileActionMngr::instance()->addToGroup(groupLeader, selectedInfos);
*/
}

void ImportIconView::createGroupByTimeFromSelection()
{
    // TODO: Implement grouping in import tool.
/*
    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();

    while (selectedInfos.size() > 0)
    {
        QList<CamItemInfo> group;
        CamItemInfo groupLeader = selectedInfos.takeFirst();
        QDateTime dateTime    = groupLeader.dateTime();

        while (selectedInfos.size() > 0 && abs(dateTime.secsTo(selectedInfos.first().dateTime())) < 2)
        {
           group.push_back(selectedInfos.takeFirst());
        }

        FileActionMngr::instance()->addToGroup(groupLeader, group);
    }
*/
}

void ImportIconView::ungroupSelected()
{
    // TODO: Implement grouping in import tool.
/*
    FileActionMngr::instance()->ungroup(selectedCamItemInfos());
*/
}

void ImportIconView::removeSelectedFromGroup()
{
    // TODO: Implement grouping in import tool.
/*
    FileActionMngr::instance()->removeFromGroup(selectedCamItemInfos());
*/
}

void ImportIconView::slotRotateLeft(const QList<QModelIndex>& /*indexes*/)
{
/*
    QList<ItemInfo> imageInfos;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        ItemInfo imageInfo(importFilterModel()->camItemInfo(index).url());
        imageInfos << imageInfo;
    }

    FileActionMngr::instance()->transform(imageInfos, MetaEngineRotation::Rotate270);
*/
}

void ImportIconView::slotRotateRight(const QList<QModelIndex>& /*indexes*/)
{
/*
    QList<ItemInfo> imageInfos;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        ItemInfo imageInfo(importFilterModel()->camItemInfo(index).url());
        imageInfos << imageInfo;
    }

    FileActionMngr::instance()->transform(imageInfos, MetaEngineRotation::Rotate90);
*/
}

void ImportIconView::activated(const CamItemInfo& info, Qt::KeyboardModifiers)
{
    if (info.isNull())
    {
        return;
    }

    int leftClickAction = ImportSettings::instance()->getItemLeftClickAction();

    if      (leftClickAction == ImportSettings::ShowPreview)
    {
        Q_EMIT previewRequested(info, false);
    }
    else if (leftClickAction == ImportSettings::StartEditor)
    {
/*      TODO
        openFile(info);
*/
    }
    else
    {
        QDesktopServices::openUrl(info.url());
    }
}

void ImportIconView::showContextMenuOnInfo(QContextMenuEvent* event, const CamItemInfo& /*info*/)
{
    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();
    QList<qlonglong>   selectedItemIDs;

    for (const CamItemInfo& info : std::as_const(selectedInfos))
    {
        selectedItemIDs << info.id;
    }

    // --------------------------------------------------------

    QMenu popmenu(this);
    ImportContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(QLatin1String("importui_fullscreen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addAction(QLatin1String("import_zoomfit2window"));
    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAction(QLatin1String("importui_imagedownload"));
    cmhelper.addAction(QLatin1String("importui_imagemarkasdownloaded"));
    cmhelper.addAction(QLatin1String("importui_imagelock"));
    cmhelper.addAction(QLatin1String("importui_delete"));
    cmhelper.addSeparator();
    cmhelper.addAction(QLatin1String("importui_item_view"));
    cmhelper.addServicesMenu(selectedUrls());

/*  TODO
    cmhelper.addRotateMenu(selectedItemIDs);
*/

    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAction(QLatin1String("importui_selectall"));
    cmhelper.addAction(QLatin1String("importui_selectnone"));
    cmhelper.addAction(QLatin1String("importui_selectinvert"));
    cmhelper.addSeparator();

    // --------------------------------------------------------
/*
    cmhelper.addAssignTagsMenu(selectedItemIDs);
    cmhelper.addRemoveTagsMenu(selectedItemIDs);
    cmhelper.addSeparator();
*/
    // --------------------------------------------------------

    cmhelper.addLabelsAction();
/*
    if (!d->faceMode)
    {
        cmhelper.addGroupMenu(selectedItemIDs);
    }

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(assignTagToSelected(int)));

    TODO: Implement tag view for import tool.

    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(removeTagFromSelected(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(gotoTagAndImageRequested(int)));
*/
    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(assignPickLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(assignColorLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(assignRatingToSelected(int)));
/*
    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(insertSelectedToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(createGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));
*/
    // --------------------------------------------------------

    cmhelper.exec(event->globalPos());
}

void ImportIconView::showContextMenu(QContextMenuEvent* event)
{
    QMenu popmenu(this);
    ImportContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(QLatin1String("importui_fullscreen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addSeparator();
    cmhelper.addAction(QLatin1String("importui_close"));

    // --------------------------------------------------------

    cmhelper.exec(event->globalPos());
}

void ImportIconView::assignTagToSelected(int tagID)
{
    CamItemInfoList infos = selectedCamItemInfos();

    for (const CamItemInfo& info : std::as_const(infos))
    {
        importItemModel()->camItemInfoRef(importItemModel()->indexForCamItemInfo(info)).tagIds.append(tagID);
    }
}

void ImportIconView::removeTagFromSelected(int tagID)
{
    CamItemInfoList infos = selectedCamItemInfos();

    for (const CamItemInfo& info : std::as_const(infos))
    {
        importItemModel()->camItemInfoRef(importItemModel()->indexForCamItemInfo(info)).tagIds.removeAll(tagID);
    }
}

void ImportIconView::assignPickLabel(const QModelIndex& index, int pickId)
{
    importItemModel()->camItemInfoRef(index).pickLabel = pickId;
}

void ImportIconView::assignPickLabelToSelected(int pickId)
{
    CamItemInfoList infos = selectedCamItemInfos();

    for (const CamItemInfo& info : std::as_const(infos))
    {
        importItemModel()->camItemInfoRef(importItemModel()->indexForCamItemInfo(info)).pickLabel = pickId;
    }
}

void ImportIconView::assignColorLabel(const QModelIndex& index, int colorId)
{
    importItemModel()->camItemInfoRef(index).colorLabel = colorId;
}

void ImportIconView::assignColorLabelToSelected(int colorId)
{
    CamItemInfoList infos = selectedCamItemInfos();

    for (const CamItemInfo& info : std::as_const(infos))
    {
        importItemModel()->camItemInfoRef(importItemModel()->indexForCamItemInfo(info)).colorLabel = colorId;
    }
}

void ImportIconView::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (index.isValid())
        {
            importItemModel()->camItemInfoRef(index).rating = rating;
        }
    }
}

void ImportIconView::assignRatingToSelected(int rating)
{
    CamItemInfoList infos = selectedCamItemInfos();

    for (const CamItemInfo& info : std::as_const(infos))
    {
        importItemModel()->camItemInfoRef(importItemModel()->indexForCamItemInfo(info)).rating = rating;
    }
}

} // namespace Digikam

#include "moc_importiconview.cpp"
