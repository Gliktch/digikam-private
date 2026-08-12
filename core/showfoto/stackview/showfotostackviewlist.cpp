/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : List-view for the Showfoto stack view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewlist.h"

// Qt includes

#include <QApplication>
#include <QActionGroup>
#include <QStyle>
#include <QIcon>
#include <QFileInfo>
#include <QHeaderView>
#include <QDir>
#include <QTimer>
#include <QMenu>
#include <QMimeData>
#include <QScrollBar>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "showfotothumbnailbar.h"
#include "showfotoiteminfo.h"
#include "showfotostackviewtooltip.h"
#include "showfotostackviewitem.h"
#include "showfotosettings.h"
#include "showfotoitemmodel.h"
#include "showfotodelegate.h"
#include "showfototooltipfiller.h"
#include "showfotofiltermodel.h"
#include "showfotothumbnailmodel.h"
#include "dfileoperations.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoStackViewList::Private
{
public:

    Private() = default;

public:

    ShowfotoStackViewSideBar* view          = nullptr;
    ShowfotoThumbnailBar*     thumbbar      = nullptr;
    QTimer*                   toolTipTimer  = nullptr;
    ShowfotoStackViewToolTip* toolTip       = nullptr;
    QModelIndex               toolTipIndex;
};

ShowfotoStackViewList::ShowfotoStackViewList(ShowfotoStackViewSideBar* const view)
    : QTreeWidget(view),
      d          (new Private)
{
    d->view     = view;

    setObjectName(QLatin1String("ShowfotoStackViewList"));
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setExpandsOnDoubleClick(false);
    setIconSize(QSize(SizeSmall, SizeSmall));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setColumnCount(4);
    QStringList titles;
    titles.append(i18nc("@title: item file name",          "Name"));
    titles.append(i18nc("@title: item file size",          "Size"));
    titles.append(i18nc("@title: item file type",          "Type"));
    titles.append(i18nc("@title: item date from metadata", "Date"));
    setHeaderLabels(titles);
    header()->setSectionResizeMode(FileName, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(FileSize, QHeaderView::Stretch);
    header()->setSectionResizeMode(FileType, QHeaderView::Stretch);
    header()->setSectionResizeMode(FileDate, QHeaderView::Stretch);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    viewport()->setMouseTracking(true);

    d->toolTip       = new ShowfotoStackViewToolTip(this);
    d->toolTipTimer  = new QTimer(this);

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));
}

ShowfotoStackViewList::~ShowfotoStackViewList()
{
    delete d->toolTip;
    delete d;
}

int ShowfotoStackViewList::sortOrder() const
{
    return d->view->sortOrder();
}

int ShowfotoStackViewList::sortRole() const
{
    return d->view->sortRole();
}

void ShowfotoStackViewList::setThumbbar(ShowfotoThumbnailBar* const thumbbar)
{
    d->thumbbar = thumbbar;

    connect(d->thumbbar->showfotoItemModel(), SIGNAL(itemInfosAboutToBeAdded(QList<ShowfotoItemInfo>)),
            this, SLOT(slotItemsAdded(QList<ShowfotoItemInfo>)));

    connect(d->thumbbar->showfotoItemModel(), SIGNAL(itemInfosAboutToBeRemoved(QList<ShowfotoItemInfo>)),
            this, SLOT(slotItemsRemoved(QList<ShowfotoItemInfo>)));

    connect(d->thumbbar->showfotoItemModel(), SIGNAL(itemInfosAdded(QList<ShowfotoItemInfo>)),
            this, SLOT(slotItemsListChanged()));

    connect(d->thumbbar->showfotoItemModel(), SIGNAL(itemInfosRemoved(QList<ShowfotoItemInfo>)),
            this, SLOT(slotItemsListChanged()));

    connect(d->thumbbar->showfotoThumbnailModel(), SIGNAL(signalItemThumbnail(ShowfotoItemInfo,QPixmap)),
            this, SLOT(slotItemThumbnail(ShowfotoItemInfo,QPixmap)));

    connect(d->thumbbar, SIGNAL(selected(QList<ShowfotoItemInfo>)),
            this, SLOT(slotItemsSelected(QList<ShowfotoItemInfo>)));

    connect(d->thumbbar, SIGNAL(deselected(QList<ShowfotoItemInfo>)),
            this, SLOT(slotItemsDeselected(QList<ShowfotoItemInfo>)));

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*)));

    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotItemActivated(QTreeWidgetItem*)));
}

void ShowfotoStackViewList::slotItemsAdded(const QList<ShowfotoItemInfo>& items)
{
    for (const ShowfotoItemInfo& info : std::as_const(items))
    {
        ShowfotoStackViewItem* const it = new ShowfotoStackViewItem(this);
        it->setInfo(info);
    }
}

void ShowfotoStackViewList::slotItemsListChanged()
{
    Q_EMIT signalItemListChanged(topLevelItemCount());
}

void ShowfotoStackViewList::slotItemsRemoved(const QList<ShowfotoItemInfo>& items)
{
    QTreeWidgetItemIterator iter(this);
    ShowfotoStackViewItem* sitem = nullptr;          // cppcheck-suppress constVariablePointer
    QList<ShowfotoStackViewItem*> list;

    while (*iter)
    {
        sitem = dynamic_cast<ShowfotoStackViewItem*>(*iter);

        if (sitem && (items.contains(sitem->info())))
        {
            list << sitem;
        }

        ++iter;
    }

    for (ShowfotoStackViewItem* const it : std::as_const(list))
    {
        delete it;
    }
}

void ShowfotoStackViewList::slotItemsSelected(const QList<ShowfotoItemInfo>& items)
{
    blockSignals(true);
    QTreeWidgetItemIterator iter(this);
    ShowfotoStackViewItem* sitem = nullptr;          // cppcheck-suppress constVariablePointer

    while (*iter)
    {
        sitem = dynamic_cast<ShowfotoStackViewItem*>(*iter);

        if (sitem && items.contains(sitem->info()))
        {
            sitem->setSelected(true);
        }

        ++iter;
    }

    blockSignals(false);
}

void ShowfotoStackViewList::slotItemsDeselected(const QList<ShowfotoItemInfo>& items)
{
    blockSignals(true);

    QTreeWidgetItemIterator iter(this);
    ShowfotoStackViewItem* sitem = nullptr;          // cppcheck-suppress constVariablePointer

    while (*iter)
    {
        sitem = dynamic_cast<ShowfotoStackViewItem*>(*iter);

        if (sitem && items.contains(sitem->info()))
        {
            sitem->setSelected(false);
        }

        ++iter;
    }

    blockSignals(false);
}

void ShowfotoStackViewList::slotSelectionChanged(QTreeWidgetItem* item)
{
    const ShowfotoStackViewItem* const sitem = dynamic_cast<ShowfotoStackViewItem*>(item);

    if (sitem)
    {
        d->thumbbar->setCurrentInfo(sitem->info());
    }
}

void ShowfotoStackViewList::slotItemActivated(QTreeWidgetItem* item)
{
    const ShowfotoStackViewItem* const sitem = dynamic_cast<ShowfotoStackViewItem*>(item);

    if (sitem)
    {
        Q_EMIT signalShowfotoItemInfoActivated(sitem->info());
    }
}

void ShowfotoStackViewList::slotItemThumbnail(const ShowfotoItemInfo& info, const QPixmap& pix)
{
    QTreeWidgetItemIterator iter(this);
    ShowfotoStackViewItem* sitem = nullptr;              // cppcheck-suppress constVariablePointer

    while (*iter)
    {
        sitem = dynamic_cast<ShowfotoStackViewItem*>(*iter);

        if (sitem && (sitem->info() == info))
        {
            sitem->setThumbnail(pix);
            return;
        }

        ++iter;
    }
}

void ShowfotoStackViewList::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    ShowfotoItemInfo info = infoFromIndex(index);

    if (!info.isNull() && index.isValid())
    {
        ShowfotoStackViewItem* const sitem = dynamic_cast<ShowfotoStackViewItem*>(itemFromIndex(index));

        if (
            sitem &&
            (
                sitem->icon(FileName).isNull() ||
                (sitem->icon(FileName).actualSize(iconSize()) != iconSize())
            )
           )
        {
            d->thumbbar->showfotoThumbnailModel()->thumbnailLoadThread()->find(ThumbnailIdentifier(info.url.toLocalFile()));
        }
    }

    QTreeWidget::drawRow(p, opt, index);
}

ShowfotoItemInfo ShowfotoStackViewList::infoFromIndex(const QModelIndex& index) const
{
    ShowfotoItemInfo info;

    if (index.isValid())
    {
        const ShowfotoStackViewItem* const sitem = dynamic_cast<ShowfotoStackViewItem*>(itemFromIndex(index));

        if (sitem)
        {
            info = sitem->info();
        }
    }

    return info;
}

void ShowfotoStackViewList::slotOpenInFileManager()
{
    QModelIndex index  = currentIndex();
    QList<QUrl> urlList;

    if (index.isValid())
    {
        const ShowfotoStackViewItem* const sitem = dynamic_cast<ShowfotoStackViewItem*>(itemFromIndex(index));

        if (sitem)
        {
            urlList << QUrl::fromLocalFile(sitem->info().url.toLocalFile());

            DFileOperations::openInFileManager(urlList);
        }
    }
}

void ShowfotoStackViewList::slotIconSizeChanged(int size)
{
    setIconSize(QSize(size, size));
}

void ShowfotoStackViewList::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu* const ctxmenu        = new QMenu(this);
    ctxmenu->setTitle(i18nc("@title", "Stack-View"));

    QMenu* const iconMenu       = new QMenu(i18nc("@title:menu", "Icon Size"), ctxmenu);
    iconMenu->setIcon(QIcon::fromTheme(QLatin1String("file-zoom-in")));

    QActionGroup* const sizeGrp = new QActionGroup(iconMenu);

    QAction* const sizeSmall    = iconMenu->addAction(i18nc("@action:inmenu", "Small (%1x%2)",  SizeSmall,  SizeSmall));
    sizeSmall->setCheckable(true);
    sizeGrp->addAction(sizeSmall);

    QAction* const sizeMedium   = iconMenu->addAction(i18nc("@action:inmenu", "Medium (%1x%2)", SizeMedium, SizeMedium));
    sizeMedium->setCheckable(true);
    sizeGrp->addAction(sizeMedium);

    QAction* const sizeLarge    = iconMenu->addAction(i18nc("@action:inmenu", "Large (%1x%2)",  SizeLarge,  SizeLarge));
    sizeLarge->setCheckable(true);
    sizeGrp->addAction(sizeLarge);

    QAction* const sizeHuge     = iconMenu->addAction(i18nc("@action:inmenu", "Huge (%1x%2)",   SizeHuge,   SizeHuge));
    sizeHuge->setCheckable(true);
    sizeGrp->addAction(sizeHuge);

    sizeGrp->setExclusive(true);

    switch (iconSize().width())
    {
        case SizeMedium:
        {
            sizeMedium->setChecked(true);
            break;
        }

        case SizeLarge:
        {
            sizeLarge->setChecked(true);
            break;
        }

        case SizeHuge:
        {
            sizeHuge->setChecked(true);
            break;
        }

        default:
        {
            sizeSmall->setChecked(true);
            break;
        }
    }

    ctxmenu->addMenu(iconMenu);
    ctxmenu->addSeparator();
    ctxmenu->addActions(d->view->pluginActions());
    ctxmenu->addSeparator();

    QAction* const addFavorite     = new QAction(QIcon::fromTheme(QLatin1String("list-add")),
                                                 i18nc("@action: context menu", "Add as Favorite"), ctxmenu);
    ctxmenu->addAction(addFavorite);

    connect(addFavorite, SIGNAL(triggered()),
            this, SIGNAL(signalAddFavorite()));

    QAction* const openFileMngr    = new QAction(QIcon::fromTheme(QLatin1String("folder-open")),
                                                 i18nc("@action: context menu", "Open in File Manager"), ctxmenu);
    ctxmenu->addAction(openFileMngr);

    connect(openFileMngr, SIGNAL(triggered()),
            this, SLOT(slotOpenInFileManager()));

    QAction* const clearListAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")),
                                                 i18nc("@action: context menu", "Clear All Items"), this);

    ctxmenu->addAction(clearListAction);

    connect(clearListAction, SIGNAL(triggered()),
            this, SIGNAL(signalClearItemsList()));

    QAction* const removeItemsAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")),
                                                   i18nc("@action: context menu", "Remove Items"), this);

    ctxmenu->addAction(removeItemsAction);

    connect(removeItemsAction, SIGNAL(triggered()),
            this, SLOT(slotRemoveItems()));

    ctxmenu->exec(e->globalPos());

    if      (sizeSmall->isChecked())
    {
        setIconSize(QSize(SizeSmall, SizeSmall));
    }
    else if (sizeMedium->isChecked())
    {
        setIconSize(QSize(SizeMedium, SizeMedium));
    }
    else if (sizeLarge->isChecked())
    {
        setIconSize(QSize(SizeLarge, SizeLarge));
    }
    else if (sizeHuge->isChecked())
    {
        setIconSize(QSize(SizeHuge, SizeHuge));
    }

    delete ctxmenu;

    QTreeView::contextMenuEvent(e);
}

void ShowfotoStackViewList::slotRemoveItems()
{
    QList<QTreeWidgetItem*> sel = selectedItems();

    if (sel.isEmpty())
    {
        return;
    }

    QList<ShowfotoItemInfo> infos;

    for (QTreeWidgetItem* const item : std::as_const(sel))
    {
        ShowfotoStackViewItem* const sitem = dynamic_cast<ShowfotoStackViewItem*>(item);

        if (sitem)
        {
            infos << sitem->info();
            delete sitem;
        }
    }

    Q_EMIT signalRemoveItemInfos(infos);
}

void ShowfotoStackViewList::hideToolTip()
{
    d->toolTipIndex = QModelIndex();
    d->toolTipTimer->stop();
    slotToolTip();
}

void ShowfotoStackViewList::slotToolTip()
{
    d->toolTip->setIndex(d->toolTipIndex);
}

bool ShowfotoStackViewList::acceptToolTip(const QModelIndex& index) const
{
    ShowfotoItemInfo info = infoFromIndex(index);

    if (!info.isNull())
    {
        return true;
    }

    return false;
}

void ShowfotoStackViewList::mouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::NoButton)
    {
        QModelIndex index = indexAt(e->pos());

        if (ShowfotoSettings::instance()->getShowToolTip())
        {
            if (!isActiveWindow())
            {
                hideToolTip();
                return;
            }

            if (index != d->toolTipIndex)
            {
                hideToolTip();

                if (acceptToolTip(index))
                {
                    d->toolTipIndex = index;
                    d->toolTipTimer->setSingleShot(true);
                    d->toolTipTimer->start(500);
                }
            }

            if ((index == d->toolTipIndex) && !acceptToolTip(index))
            {
                hideToolTip();
            }
        }

        return;
    }

    hideToolTip();
    QTreeView::mouseMoveEvent(e);
}

void ShowfotoStackViewList::wheelEvent(QWheelEvent* e)
{
    hideToolTip();
    QTreeView::wheelEvent(e);
}

void ShowfotoStackViewList::keyPressEvent(QKeyEvent* e)
{
    hideToolTip();
    QTreeView::keyPressEvent(e);
}

void ShowfotoStackViewList::focusOutEvent(QFocusEvent* e)
{
    hideToolTip();
    QTreeView::focusOutEvent(e);
}

void ShowfotoStackViewList::leaveEvent(QEvent* e)
{
    hideToolTip();
    QTreeView::leaveEvent(e);
}

QList<QUrl> ShowfotoStackViewList::urls()
{
    QTreeWidgetItemIterator iter(this);
    const ShowfotoStackViewItem* sitem = nullptr;
    QList<QUrl> list;

    while (*iter)
    {
        sitem = dynamic_cast<ShowfotoStackViewItem*>(*iter);

        if (sitem)
        {
            list << sitem->info().url;
        }

        ++iter;
    }

    return list;
}

QUrl ShowfotoStackViewList::currentUrl() const
{
    const ShowfotoStackViewItem* sitem = dynamic_cast<ShowfotoStackViewItem*>(currentItem());

    if (sitem)
    {
        return (sitem->info().url);
    }

    return QUrl();
}

} // namespace ShowFoto

#include "moc_showfotostackviewlist.cpp"
