/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : Trash view
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "trashview.h"

// Qt includes

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QToolButton>
#include <QPushButton>
#include <QHeaderView>
#include <QListView>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QUrl>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dtrashiteminfo.h"
#include "dtrashitemmodel.h"
#include "thumbnailsize.h"
#include "dio.h"

namespace Digikam
{

class Q_DECL_HIDDEN TrashView::Private
{

public:

    Private() = default;

public:

    DTrashItemModel*               model                    = nullptr;
    ThumbnailAligningDelegate*     thumbDelegate            = nullptr;
    QVBoxLayout*                   mainLayout               = nullptr;
    QHBoxLayout*                   btnsLayout               = nullptr;
    QTableView*                    tableView                = nullptr;
    QPushButton*                   undoButton               = nullptr;
    QPushButton*                   deleteButton             = nullptr;
    QAction*                       restoreAction            = nullptr;
    QAction*                       deleteAction             = nullptr;
    QAction*                       deleteAllAction          = nullptr;

    QModelIndex                    lastSelectedIndex;

    QHash<QString, DTrashItemInfo> lastTrashItemCache;

    DTrashItemInfo                 lastSelectedItem;
    QModelIndexList                selectedIndexesToRemove;
    ThumbnailSize                  thumbSize                = ThumbnailSize(ThumbnailSize::Large);
};

TrashView::TrashView(QWidget* const parent)
    : QWidget          (parent),
      StateSavingObject(this),
      d                (new Private)
{
    setObjectName(QLatin1String("TrashView"));

    // Layouts

    d->mainLayout    = new QVBoxLayout(this);
    d->btnsLayout    = new QHBoxLayout();

    // View and tools

    d->tableView     = new QTableView(this);
    d->model         = new DTrashItemModel(this, this);
    d->thumbDelegate = new ThumbnailAligningDelegate(this);

    // Table view settings

    d->tableView->setModel(d->model);
    d->tableView->setItemDelegateForColumn(0, d->thumbDelegate);
    d->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    d->tableView->horizontalHeader()->setStretchLastSection(true);                      // Stretch if no column size config are not found in settings file.
    d->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    d->tableView->verticalHeader()->setDefaultSectionSize(d->thumbSize.size());
    d->tableView->verticalHeader()->hide();
    d->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tableView->setShowGrid(false);
    d->tableView->setSortingEnabled(true);
    d->tableView->sortByColumn(DTrashItemModel::DTrashTimeStamp, Qt::DescendingOrder);
    d->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Action Buttons

    QPushButton* const helpButton = new QPushButton(QIcon::fromTheme(QLatin1String("help-browser")), i18n("Help"));
    helpButton->setToolTip(i18nc("@info", "Online help about trash-bin"));

    connect(helpButton, &QPushButton::clicked,
            this, []()
        {
            openOnlineDocumentation(QLatin1String("main_window"),
                                    QLatin1String("image_view"),
                                    QLatin1String("deleting-photograph"));
        }
    );

    d->undoButton      = new QPushButton(i18n("Undo"), this);
    d->undoButton->setToolTip(i18n("Restore only the last entry in the trash-bin."));
    d->undoButton->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));
    d->undoButton->setEnabled(false);

    QToolButton* const restoreButton = new QToolButton(this);
    d->restoreAction                 = new QAction(i18n("Restore"), this);
    d->restoreAction->setToolTip(i18n("Restore selection of files from the trash-bin."));
    d->restoreAction->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    d->restoreAction->setEnabled(false);
    restoreButton->setDefaultAction(d->restoreAction);
    restoreButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    d->deleteButton    = new QPushButton(i18n("Delete..."), this);
    d->deleteButton->setToolTip(i18n("Remove permanently the items selection or all items from the trash-bin."));
    d->deleteButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->deleteButton->setEnabled(false);

    d->deleteAction    = new QAction(i18n("Selected Items Permanently"), this);
    d->deleteAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));

    d->deleteAllAction = new QAction(i18n("All Items Permanently"),      this);
    d->deleteAllAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));

    QMenu* const menu  = new QMenu(this);
    menu->addAction(d->deleteAction);
    menu->addAction(d->deleteAllAction);

    d->deleteButton->setMenu(menu);

    // Adding widgets to layouts

    d->mainLayout->addWidget(d->tableView);

    d->btnsLayout->addWidget(helpButton);
    d->btnsLayout->addWidget(d->undoButton);
    d->btnsLayout->addStretch();
    d->btnsLayout->addWidget(restoreButton);
    d->btnsLayout->addStretch();
    d->btnsLayout->addWidget(d->deleteButton);
    d->btnsLayout->setStretchFactor(restoreButton, 5);

    d->mainLayout->addLayout(d->btnsLayout);
    d->mainLayout->setContentsMargins(QMargins());

    // Signals and Slots connections

    connect(d->undoButton, SIGNAL(released()),
            this, SLOT(slotUndoLastDeletedItems()));

    connect(d->restoreAction, SIGNAL(triggered()),
            this, SLOT(slotRestoreSelectedItems()));

    connect(d->deleteAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteSelectedItems()));

    connect(this, SIGNAL(signalEmptytrash()),
            this, SLOT(slotDeleteAllItems()));

    connect(d->deleteAllAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteAllItems()));

    connect(d->model, SIGNAL(dataChange()),
            this, SLOT(slotDataChanged()));

    connect(d->model, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->model, SIGNAL(signalLoadingFinished()),
            this, SLOT(slotLoadingFinished()));

    connect(d->tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotChangeLastSelectedItem(QModelIndex,QModelIndex)));

    connect(d->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged()));

    connect(d->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenuEmptyTrash(QPoint)));

    loadState();
}

TrashView::~TrashView()
{
    saveState();

    delete d;
}

DTrashItemModel* TrashView::model() const
{
    return d->model;
}

ThumbnailSize TrashView::getThumbnailSize() const
{
    return d->thumbSize;
}

void TrashView::slotSelectionChanged()
{
    if (d->tableView->selectionModel()->hasSelection())
    {
        d->restoreAction->setEnabled(true);
        d->deleteAction->setEnabled(true);
    }
    else
    {
        d->restoreAction->setEnabled(false);
        d->deleteAction->setEnabled(false);
    }

    Q_EMIT itemSelectionChanged();
}

void TrashView::slotUndoLastDeletedItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Undo last deleted items from collection trash";

    DTrashItemInfoList items;
    d->selectedIndexesToRemove.clear();
    QDateTime lastDateTime = QDateTime::fromMSecsSinceEpoch(0);

    {
        const auto its       = d->model->allItems();

        for (const DTrashItemInfo& item : its)
        {
            if (item.deletionTimestamp > lastDateTime)
            {
                // cppcheck-suppress useStlAlgorithm
                lastDateTime = item.deletionTimestamp;
            }
        }
    }

    {
        const auto its = d->model->allItems();

        for (const DTrashItemInfo& item : its)
        {
            if (item.deletionTimestamp == lastDateTime)
            {
                QModelIndex index = d->model->indexForItem(item);

                if (index.isValid())
                {
                    items << item;
                    d->selectedIndexesToRemove << index;
                }
            }
        }
    }

    if (items.isEmpty())
    {
        return;
    }

    QString title = i18n("Confirm Undo");
    QString msg   = i18np("Are you sure you want to restore %1 item?",
                          "Are you sure you want to restore %1 items?", items.count());
    int result    = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items to Restore:\n " << items;

    d->model->suspendLoadingTrashThumbs();

    DIO::restoreTrash(items);

    connect(DIO::instance(), SIGNAL(signalTrashFinished()),
            this, SLOT(slotRemoveItemsFromModel()));
}

void TrashView::slotRestoreSelectedItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Restoring selected items from collection trash";

    d->selectedIndexesToRemove = d->tableView->selectionModel()->selectedRows();
    DTrashItemInfoList items   = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    QString title = i18n("Confirm Restore");
    QString msg   = i18np("Are you sure you want to restore %1 item?",
                          "Are you sure you want to restore %1 items?", items.count());
    int result    = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items to Restore:\n " << items;

    d->model->suspendLoadingTrashThumbs();

    DIO::restoreTrash(items);

    connect(DIO::instance(), SIGNAL(signalTrashFinished()),
            this, SLOT(slotRemoveItemsFromModel()));
}

void TrashView::slotDeleteSelectedItems()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Deleting selected items from collection trash";

    d->selectedIndexesToRemove = d->tableView->selectionModel()->selectedRows();

    if (d->selectedIndexesToRemove.isEmpty())
    {
        return;
    }

    QString title = i18n("Confirm Deletion");
    QString msg   = i18np("Are you sure you want to delete %1 item permanently?",
                          "Are you sure you want to delete %1 items permanently?",
                          d->selectedIndexesToRemove.count());
    int result    = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
    {
        return;
    }

    DTrashItemInfoList items = d->model->itemsForIndexes(d->selectedIndexesToRemove);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Items count: " << items.count();

    d->model->suspendLoadingTrashThumbs();

    DIO::emptyTrash(items);

    connect(DIO::instance(), SIGNAL(signalTrashFinished()),
            this, SLOT(slotRemoveItemsFromModel()));
}

void TrashView::slotRemoveItemsFromModel()
{
    disconnect(DIO::instance(), nullptr, this, nullptr);

    if (d->selectedIndexesToRemove.isEmpty())
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Removing deleted items from view";

    d->model->removeItems(d->selectedIndexesToRemove);
    d->selectedIndexesToRemove.clear();
}

void TrashView::slotRemoveAllItemsFromModel()
{
    disconnect(DIO::instance(), nullptr, this, nullptr);

    d->model->clearCurrentData();
}

void TrashView::slotDeleteAllItems()
{
    if (d->model->isEmpty())
    {
        return;
    }

    QString title = i18n("Confirm Deletion");
    QString msg   = i18n("Are you sure you want to delete ALL items permanently?");
    int result    = QMessageBox::warning(this, title, msg, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::No)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Removing all item from trash permanently";

    d->model->suspendLoadingTrashThumbs();

    DIO::emptyTrash(d->model->allItems());

    connect(DIO::instance(), SIGNAL(signalTrashFinished()),
            this, SLOT(slotRemoveAllItemsFromModel()));
}

void TrashView::slotDataChanged()
{
    selectLastSelected();

    if (d->model->isEmpty())
    {
        d->undoButton->setEnabled(false);
        d->deleteButton->setEnabled(false);
        d->restoreAction->setEnabled(false);

        return;
    }

    d->undoButton->setEnabled(true);
    d->deleteButton->setEnabled(true);
}

void TrashView::slotLoadingStarted()
{
    if (!d->model->trashAlbumPath().isEmpty() && !d->lastSelectedItem.isNull())
    {
        d->lastTrashItemCache[d->model->trashAlbumPath()] = d->lastSelectedItem;
    }
}

void TrashView::slotLoadingFinished()
{
    if (!d->model->trashAlbumPath().isEmpty())
    {
        DTrashItemInfo item = d->lastTrashItemCache.value(d->model->trashAlbumPath());
        QModelIndex index   = d->model->indexForItem(item);

        if (index.isValid())
        {
            d->lastSelectedIndex = index;
            d->lastSelectedItem  = item;
        }
    }

    selectLastSelected();
}

void TrashView::slotChangeLastSelectedItem(const QModelIndex& curr, const QModelIndex&)
{
    d->lastSelectedIndex = curr;
    d->lastSelectedItem  = d->model->itemForIndex(curr);

    Q_EMIT itemSelectionChanged();
}

void TrashView::slotContextMenuEmptyTrash(const QPoint& pos)
{
    QMenu* const emptyTrashMenu = new QMenu(this);
    emptyTrashMenu->addSection(i18nc("delete the selected items from trash", "Selection"));
    emptyTrashMenu->addAction(d->restoreAction);
    emptyTrashMenu->addSection(i18nc("delete all items from trash", "Delete"));
    emptyTrashMenu->addAction(d->deleteAllAction);
    emptyTrashMenu->addAction(d->deleteAction);
    emptyTrashMenu->popup(d->tableView->viewport()->mapToGlobal(pos));

    return;
}

void TrashView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    d->model->changeThumbSize(thumbSize.size());
    d->tableView->verticalHeader()->setDefaultSectionSize(thumbSize.size());
    d->thumbSize = thumbSize;
}

QUrl TrashView::lastSelectedItemUrl() const
{
    return QUrl::fromLocalFile(d->lastSelectedItem.trashPath);
}

void TrashView::selectLastSelected()
{
    if      (d->model->isEmpty())
    {
        d->lastSelectedIndex = QModelIndex();
    }
    else if (!d->lastSelectedIndex.isValid())
    {
        d->tableView->selectRow(0);
        d->lastSelectedIndex = d->model->index(0, 0);
    }
    else
    {
        d->tableView->selectRow(d->lastSelectedIndex.row());
        d->lastSelectedIndex = d->model->index(d->lastSelectedIndex.row(), 0);
    }

    if (!d->lastSelectedIndex.isValid())
    {
        d->lastSelectedItem = DTrashItemInfo();
    }
    else
    {
        d->lastSelectedItem = d->model->itemForIndex(d->lastSelectedIndex);
        d->tableView->scrollTo(d->lastSelectedIndex, QAbstractItemView::EnsureVisible);
    }

    Q_EMIT itemSelectionChanged();
}

QString TrashView::statusBarText() const
{
    int selectionCount = d->tableView->selectionModel()->selectedRows().count();
    int numberOfItems  = d->model->rowCount(QModelIndex());

    QString statusBarSelectionText;

    if (selectionCount == 0)
    {
        statusBarSelectionText
                = i18np("No item selected (%1 item)",
                        "No item selected (%1 items)",
                        numberOfItems);
    }
    else
    {
        statusBarSelectionText
                = i18n("%1/%2 items selected",
                       selectionCount, numberOfItems);
    }

    return statusBarSelectionText;
}

void TrashView::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    QHeaderView* const header = d->tableView->horizontalHeader();

    for (int i = 0 ; i < header->count() ; ++i)
    {
        header->resizeSection(i, group.readEntry(QString::fromLatin1("TrashViewColumn_%1").arg(i), header->sectionSize(i)));
    }
}

void TrashView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    QHeaderView* const header = d->tableView->horizontalHeader();

    for (int i = 0 ; i < header->count() ; ++i)
    {
        group.writeEntry(QString::fromLatin1("TrashViewColumn_%1").arg(i), header->sectionSize(i));
    }

    group.sync();
}

// --------------------------------------------------

ThumbnailAligningDelegate::ThumbnailAligningDelegate(QObject* const parent)
    : QStyledItemDelegate(parent)
{
}

void ThumbnailAligningDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    QPixmap pixmap;
    pixmap        = index.data(Qt::DecorationRole).value<QPixmap>();
    QSize pixSize = (QSizeF(pixmap.size()) / pixmap.devicePixelRatio()).toSize();
    QPoint loc    = option.rect.center() - QRect(0, 0, pixSize.width(), pixSize.height()).center();

    painter->save();

    if      (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->fillRect(option.rect, option.palette.brush(QPalette::Inactive, QPalette::Highlight));
    }

    painter->drawPixmap(loc, pixmap);
    painter->restore();
}

} // namespace Digikam

#include "moc_trashview.cpp"
