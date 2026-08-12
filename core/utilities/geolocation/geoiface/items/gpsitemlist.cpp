/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A view to display a list of items with GPS info.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsitemlist.h"

// Qt includes

#include <QDrag>
#include <QHeaderView>
#include <QWheelEvent>
#include <QMenu>
#include <QAction>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "gpsitemdelegate.h"
#include "gpsitemlistdragdrophandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN GPSItemList::Private
{
public:

    Private() = default;

    bool                     editEnabled            = true;
    bool                     dragEnabled            = false;
    GPSItemModel*            model                  = nullptr;
    QItemSelectionModel*     selectionModel         = nullptr;
    GPSItemDelegate*         itemDelegate           = nullptr;
    GPSItemSortProxyModel*   imageSortProxyModel    = nullptr;
    ItemListDragDropHandler* dragDropHandler        = nullptr;
};

GPSItemList::GPSItemList(QWidget* const parent)
    : QTreeView(parent),
      d        (new Private)
{
    header()->setSectionsMovable(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setRootIsDecorated(false);

    d->itemDelegate = new GPSItemDelegate(this, this);
    setItemDelegate(d->itemDelegate);
    setThumbnailSize(60);
    slotUpdateActionsEnabled();

    header()->installEventFilter(this);
}

GPSItemList::~GPSItemList()
{
    delete d;
}

void GPSItemList::startDrag(Qt::DropActions supportedActions)
{
    if (!d->dragDropHandler)
    {
        QTreeView::startDrag(supportedActions);

        return;
    }

    // NOTE: read the selected indices from the source selection model, not our selection model,
    // which is for the sorted model!

    const QList<QModelIndex> selectedIndicesFromModel = d->selectionModel->selectedIndexes();
    QList<QPersistentModelIndex> selectedIndices;

    for (int i = 0 ; i < selectedIndicesFromModel.count() ; ++i)
    {
        selectedIndices << selectedIndicesFromModel.at(i);
    }

    QMimeData* const dragMimeData = d->dragDropHandler->createMimeData(selectedIndices);

    if (!dragMimeData)
    {
        return;
    }

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(dragMimeData);
    drag->exec(Qt::CopyAction);
}

void GPSItemList::setModelAndSelectionModel(GPSItemModel* const model, QItemSelectionModel* const selectionModel)
{
    d->model               = model;
    d->selectionModel      = selectionModel;
    d->imageSortProxyModel = new GPSItemSortProxyModel(d->model, d->selectionModel);
    setModel(d->imageSortProxyModel);

    connect(d->model, SIGNAL(signalThumbnailForIndexAvailable(QPersistentModelIndex,QPixmap)),
            this, SLOT(slotThumbnailFromModel(QPersistentModelIndex,QPixmap)));

    connect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotInternalTreeViewImageActivated(QModelIndex,QModelIndex)));

    if (d->imageSortProxyModel->mappedSelectionModel())
    {
        setSelectionModel(d->imageSortProxyModel->mappedSelectionModel());
    }
}

void GPSItemList::setDragDropHandler(ItemListDragDropHandler* const dragDropHandler)
{
    d->dragDropHandler = dragDropHandler;
}

GPSItemModel* GPSItemList::getModel() const
{
    return d->model;
}

void GPSItemList::setThumbnailSize(const int size)
{
    d->itemDelegate->setThumbnailSize(size);
    setColumnWidth(GPSItemContainer::ColumnThumbnail, size);
}

void GPSItemList::slotIncreaseThumbnailSize()
{
    // TODO: pick reasonable limits and make sure we stay on multiples of 5

    const int currentThumbnailSize = d->itemDelegate->getThumbnailSize();

    if (currentThumbnailSize < 200)
    {
        setThumbnailSize(currentThumbnailSize + 5);
    }
}

void GPSItemList::slotDecreaseThumbnailSize()
{
    const int currentThumbnailSize = d->itemDelegate->getThumbnailSize();

    if (currentThumbnailSize > 30)
    {
        setThumbnailSize(currentThumbnailSize - 5);
    }
}

void GPSItemList::wheelEvent(QWheelEvent* we)
{
    if ((we->modifiers() & Qt::ControlModifier) == 0)
    {
        QTreeView::wheelEvent(we);

        return;
    }

    we->accept();

    if      (we->angleDelta().y() > 0)
    {
        slotIncreaseThumbnailSize();
    }
    else if (we->angleDelta().y() < 0)
    {
        slotDecreaseThumbnailSize();
    }
}

void GPSItemList::slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& /*pixmap*/)
{
    // TODO: verify that the size corresponds to the size of our thumbnails!

    update(d->imageSortProxyModel->mapFromSource(index));
}

void GPSItemList::saveSettingsToGroup(KConfigGroup* const group)
{
    group->writeEntry("Image List Thumbnail Size", d->itemDelegate->getThumbnailSize());
    group->writeEntry("Header State",              header()->saveState().toBase64());
}

void GPSItemList::readSettingsFromGroup(const KConfigGroup* const group)
{
    setThumbnailSize(group->readEntry("Image List Thumbnail Size", 60));

    QByteArray headerState;
    headerState = group->readEntry("Header State", headerState);

    if (!headerState.isEmpty())
    {
        header()->restoreState(QByteArray::fromBase64(headerState));
    }
    else
    {
        // by default, hide the advanced columns:

        header()->setSectionHidden(GPSItemContainer::ColumnDOP,         true);
        header()->setSectionHidden(GPSItemContainer::ColumnFixType,     true);
        header()->setSectionHidden(GPSItemContainer::ColumnNSatellites, true);
    }
}

QItemSelectionModel* GPSItemList::getSelectionModel() const
{
    return d->selectionModel;
}

void GPSItemList::slotInternalTreeViewImageActivated(const QModelIndex& current,
                                                     const QModelIndex& /*previous*/)
{
    // qCDebug(DIGIKAM_GENERAL_LOG) << current;

    Q_EMIT signalImageActivated(current);
}

GPSItemSortProxyModel* GPSItemList::getSortProxyModel() const
{
    return d->imageSortProxyModel;
}

void GPSItemList::setEditEnabled(const bool state)
{
    d->editEnabled = state;
    slotUpdateActionsEnabled();
}

void GPSItemList::setDragEnabled(const bool state)
{
    d->dragEnabled = state;
    slotUpdateActionsEnabled();
}

void GPSItemList::slotUpdateActionsEnabled()
{
    QTreeView::setDragEnabled(d->dragEnabled && d->editEnabled);

    if (d->dragEnabled && d->editEnabled)
    {
        QTreeView::setDragDropMode(QAbstractItemView::DragOnly);
    }
}

bool GPSItemList::eventFilter(QObject* watched, QEvent* event)
{
    QHeaderView* const headerView = header();

    if (!d->model || (watched != headerView) || (event->type() != QEvent::ContextMenu))
    {
        return QWidget::eventFilter(watched, event);
    }

    QMenu* const menu               = new QMenu(this);

    // add action for all the columns

    for (int i = 0 ; i < d->model->columnCount() ; ++i)
    {
        const QString columnName    = d->model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        const bool isVisible        = !headerView->isSectionHidden(i);

        QAction* const columnAction = new QAction(columnName, menu);
        columnAction->setCheckable(true);
        columnAction->setChecked(isVisible);
        columnAction->setData(i);

        menu->addAction(columnAction);
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotColumnVisibilityActionTriggered(QAction*)));

    QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
    menu->exec(e->globalPos());

    return true;
}

void GPSItemList::slotColumnVisibilityActionTriggered(QAction* action)
{
    const int columnNumber     = action->data().toInt();
    const bool columnIsVisible = action->isChecked();

    header()->setSectionHidden(columnNumber, !columnIsVisible);
}

} // namespace Digikam

#include "moc_gpsitemlist.cpp"
