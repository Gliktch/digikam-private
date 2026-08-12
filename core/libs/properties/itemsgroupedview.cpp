/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-04-11
 * Description : A list view to show grouped items
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemsgroupedview.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QHeaderView>
#include <QIcon>
#include <QTimer>
#include <QPixmap>
#include <QModelIndex>
#include <QFileInfo>

// Local includes

#include "digikam_globals.h"
#include "applicationsettings.h"
#include "itemsgroupedviewitem.h"
#include "itemsgroupedtooltip.h"
#include "itemsgroupedtoolbar.h"
#include "itemfiltermodel.h"
#include "fileactionmngr.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemsGroupedView::Private
{
public:

    Private() = default;

public:

    ThumbnailLoadThread*     thumbLoadThread     = nullptr;
    int                      iconSize            = 0;
    bool                     showCount           = false;
    bool                     showTips            = false;
    ItemFilterModel*         itemModel           = nullptr;

    ItemsGroupedToolTip*     toolTip             = nullptr;
    ItemsGroupedViewItem*    toolTipItem         = nullptr;
    QTimer*                  toolTipTimer        = nullptr;

    ItemsGroupedToolBar*     toolBar             = nullptr;
    ItemsGroupedViewItem*    toolBarItem         = nullptr;
};

ItemsGroupedView::ItemsGroupedView(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    setSortingEnabled(false);
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setColumnCount(2);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    viewport()->setMouseTracking(true);
    header()->hide();

    d->thumbLoadThread = new ThumbnailLoadThread();
    d->thumbLoadThread->setThumbnailSize(d->iconSize);
    d->thumbLoadThread->setSendSurrogatePixmap(false);

    d->toolTip         = new ItemsGroupedToolTip(this);
    d->toolTipTimer    = new QTimer(this);
    d->toolBar         = new ItemsGroupedToolBar(this);

    // ---

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription,QPixmap)),
            Qt::QueuedConnection);

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    connect(d->toolBar, &ItemsGroupedToolBar::signalDeleteRequested,
            this, [this]()
        {
            FileActionMngr::instance()->removeFromGroup(ItemInfoList() << d->toolBarItem->info());
        }
    );

    slotSettingsChanged();
}

ItemsGroupedView::~ItemsGroupedView()
{
    delete d->toolTip;
    delete d->thumbLoadThread;
    delete d;
}

void ItemsGroupedView::setItemFilterModel(ItemFilterModel* const model)
{
    d->itemModel = model;

    connect(d->itemModel, &ItemFilterModel::signalGroupIsOpen,
            this, [this](qlonglong group, bool open)
        {
            QTreeWidgetItemIterator it(this);

            while (*it)
            {
                ItemsGroupedViewItem* const item = dynamic_cast<ItemsGroupedViewItem*>(*it);

                if (item && (item->info().id() == group))
                {
                    blockSignals(true);
                    item->setExpanded(open);
                    blockSignals(false);
                }

                break;
            }

            ++it;
        }
    );

    connect(this, &QTreeWidget::itemCollapsed,
            [this](QTreeWidgetItem* item)
        {
            const ItemsGroupedViewItem* const gitem = dynamic_cast<ItemsGroupedViewItem*>(item);

            if (gitem)
            {
                d->itemModel->setGroupOpen(gitem->info().id(), false);
            }
        }
    );

    connect(this, &QTreeWidget::itemExpanded,
            [this](QTreeWidgetItem* item)
        {
            const ItemsGroupedViewItem* const gitem = dynamic_cast<ItemsGroupedViewItem*>(item);

            if (gitem)
            {
                d->itemModel->setGroupOpen(gitem->info().id(), true);
            }
        }
    );
}

ItemFilterModel* ItemsGroupedView::itemFilterModel() const
{
    return d->itemModel;
}

void ItemsGroupedView::setEnableToolTips(bool val)
{
    d->showTips = val;

    if (!val)
    {
        hideToolTip();
    }
}

void ItemsGroupedView::hideToolTip()
{
    d->toolTipItem = nullptr;
    d->toolTipTimer->stop();
    slotToolTip();
}

void ItemsGroupedView::hideToolBar()
{
    d->toolBarItem = nullptr;
    d->toolBar->hide();
}

bool ItemsGroupedView::acceptToolTip(const QPoint& pos) const
{
    if (columnAt(pos.x()) == 0)
    {
        return true;
    }

    return false;
}

bool ItemsGroupedView::acceptToolBar(const QPoint& pos) const
{
    if (columnAt(pos.x()) == 1)
    {
        return true;
    }

    return false;
}

void ItemsGroupedView::slotToolTip()
{
    d->toolTip->setItem(d->toolTipItem);
}

void ItemsGroupedView::mouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::NoButton)
    {
        ItemsGroupedViewItem* const item = dynamic_cast<ItemsGroupedViewItem*>(itemAt(e->pos()));

        // ToolTip

        if (d->showTips)
        {
            if (!isActiveWindow())
            {
                hideToolTip();
                return;
            }

            if (item != d->toolTipItem)
            {
                hideToolTip();

                if (acceptToolTip(e->pos()))
                {
                    d->toolTipItem = item;
                    d->toolTipTimer->setSingleShot(true);
                    d->toolTipTimer->start(500);
                }
            }

            if ((item == d->toolTipItem) && !acceptToolTip(e->pos()))
            {
                hideToolTip();
            }
        }

        // ToolBar

        if (!isActiveWindow())
        {
            hideToolBar();
            return;
        }

        if (item != d->toolBarItem)
        {
            hideToolBar();

            if (acceptToolBar(e->pos()))
            {
                d->toolBarItem = item;
                QRect rect     = visualItemRect(d->toolBarItem);
                QPoint pos(rect.right() - d->toolBar->width(), rect.top());
                d->toolBar->move(pos);
                d->toolBar->show();
            }
        }

        if ((item == d->toolBarItem) && !acceptToolBar(e->pos()))
        {
            hideToolBar();
        }

        return;
    }

    hideToolTip();
    hideToolBar();
    QTreeWidget::mouseMoveEvent(e);
}

void ItemsGroupedView::wheelEvent(QWheelEvent* e)
{
    hideToolTip();
    hideToolBar();
    QTreeWidget::wheelEvent(e);
}

void ItemsGroupedView::keyPressEvent(QKeyEvent* e)
{
    hideToolTip();
    hideToolBar();
    QTreeWidget::keyPressEvent(e);
}

void ItemsGroupedView::focusOutEvent(QFocusEvent* e)
{
    hideToolTip();
    hideToolBar();
    QTreeWidget::focusOutEvent(e);
}

void ItemsGroupedView::leaveEvent(QEvent* e)
{
    hideToolTip();
    hideToolBar();
    QTreeWidget::leaveEvent(e);
}

void ItemsGroupedView::setGroups(const ItemInfoList& items)
{
    clear();

    QList<ThumbnailIdentifier> thumbs;

    for (const ItemInfo& pinf : std::as_const(items))
    {
        ItemsGroupedViewItem* const p = new ItemsGroupedViewItem(this, pinf);
        addTopLevelItem(p);
        thumbs.append(ThumbnailIdentifier(pinf.fileUrl().toLocalFile()));

        if (d->itemModel)
        {
            QModelIndex index = d->itemModel->indexForItemInfo(pinf);

            if (index.isValid())
            {
                blockSignals(true);
                p->setExpanded(index.data(ItemFilterModel::GroupIsOpenRole).toBool());
                blockSignals(false);
            }
        }

        const auto list               = pinf.groupedImages();

        for (const ItemInfo& cinf : std::as_const(list))
        {
            new ItemsGroupedViewItem(p, cinf);
            thumbs.append(ThumbnailIdentifier(cinf.fileUrl().toLocalFile()));
        }
    }

    for (const ThumbnailIdentifier& th : std::as_const(thumbs))
    {
        d->thumbLoadThread->find(th);
    }
}

void ItemsGroupedView::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    QPixmap thumb = pix;

    if (pix.isNull())
    {
        thumb = QIcon::fromTheme(QLatin1String("view-preview")).pixmap(d->iconSize, QIcon::Disabled);
    }

    QString file = QFileInfo(desc.filePath).fileName();
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ItemsGroupedViewItem* const item = dynamic_cast<ItemsGroupedViewItem*>(*it);

        if (item && (item->info().name() == file))
        {
            item->setThumb(thumb);
            break;
        }

        ++it;
    }
}

void ItemsGroupedView::slotSettingsChanged()
{
    if (
        d->iconSize  != ApplicationSettings::instance()->getTreeViewIconSize() ||
        d->showCount != ApplicationSettings::instance()->getShowFolderTreeViewItemsCount()
       )
    {
        d->iconSize  = ApplicationSettings::instance()->getTreeViewIconSize();
        d->showCount = ApplicationSettings::instance()->getShowFolderTreeViewItemsCount();
        setIconSize(QSize(d->iconSize, d->iconSize));
        d->thumbLoadThread->setThumbnailSize(d->iconSize);

        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            (*it)->setSizeHint(0, QSize(d->iconSize, d->iconSize));

            ItemsGroupedViewItem* const item = dynamic_cast<ItemsGroupedViewItem*>(*it);

            if (item)
            {
                item->updateTitle();
            }

            ++it;
        }
    }

    if (d->showTips != ApplicationSettings::instance()->getShowToolTips())
    {
        setEnableToolTips(ApplicationSettings::instance()->getShowToolTips());
    }
}

} // namespace Digikam

#include "moc_itemsgroupedview.cpp"
