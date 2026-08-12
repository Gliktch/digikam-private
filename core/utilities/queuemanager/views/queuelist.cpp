/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "queuelist.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QDrag>
#include <QMenu>
#include <QAction>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "ddragobjects.h"
#include "defaultrenameparser.h"
#include "queuemgrwindow.h"
#include "queuesettings.h"
#include "queuetooltip.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "workflowmanager.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class Q_DECL_HIDDEN QueueListViewItem::Private
{

public:

    Private() = default;

public:

    bool           isBusy           = false;
    bool           done             = false;
    bool           hasThumb         = false;

    int            progressIndex    = 0;

    QString        destFileName;

    QPixmap        preview;

    QueueListView* view             = nullptr;

    ItemInfo       info;
};

QueueListViewItem::QueueListViewItem(QueueListView* const view, const ItemInfo& info)
    : QTreeWidgetItem(view),
      d              (new Private)
{
    d->view = view;
    setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(48, QIcon::Disabled), false);
    setInfo(info);
}

QueueListViewItem::~QueueListViewItem()
{
    delete d;
}

bool QueueListViewItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

void QueueListViewItem::setInfo(const ItemInfo& _info)
{
    d->info = _info;
    setText(1, d->info.name());
}

ItemInfo QueueListViewItem::info() const
{
    return d->info;
}

void QueueListViewItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);

    //  We make sure the preview icon stays the same regardless of the role

    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);
}

void QueueListViewItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    QSize iSize = treeWidget()->iconSize();
    QPixmap pixmap(iSize.width() + 2, iSize.height() + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap(
                 (pixmap.width()  / 2) - (pix.width()  / 2),
                 (pixmap.height() / 2) - (pix.height() / 2),
                 pix
                );
    d->preview  = pixmap;
    setPixmap(d->preview);
    d->hasThumb = hasThumb;
}

void QueueListViewItem::animProgress()
{
    QPixmap icon(d->view->progressPixmapForIndex(d->progressIndex));
    d->progressIndex++;

    if (d->view->progressPixmapForIndex(d->progressIndex).isNull())
    {
        d->progressIndex = 0;
    }

    QPixmap preview = d->preview;
    QPixmap mask(preview.size());
    mask.fill(QColor(128, 128, 128, 192));
    QPainter p(&preview);
    p.drawPixmap(0, 0, mask);
    p.drawPixmap(
                 (preview.width()  / 2) - (icon.width()  / 2),
                 (preview.height() / 2) - (icon.height() / 2),
                 icon
                );
    setPixmap(preview);
}

void QueueListViewItem::setCanceled()
{
    setPixmap(d->preview);
    setIcon(1, QIcon::fromTheme(QLatin1String("dialog-cancel")));
    d->done          = false;
    d->isBusy        = false;
    d->progressIndex = 0;
}

void QueueListViewItem::setFailed()
{
    setPixmap(d->preview);
    setIcon(1, QIcon::fromTheme(QLatin1String("dialog-error")));
    d->done          = false;
    d->isBusy        = false;
    d->progressIndex = 0;
}

void QueueListViewItem::setDone()
{
    setPixmap(d->preview);
    setIcon(1, QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->done          = true;
    d->isBusy        = false;
    d->progressIndex = 0;
}

bool QueueListViewItem::isDone() const
{
    return d->done;
}

void QueueListViewItem::reset()
{
    setPixmap(d->preview);
    setIcon(1, QIcon());
    d->done          = false;
    d->isBusy        = false;
    d->progressIndex = 0;
}

void QueueListViewItem::setBusy()
{
    d->isBusy = true;
}

bool QueueListViewItem::isBusy() const
{
    return d->isBusy;
}

void QueueListViewItem::setDestFileName(const QString& str)
{
    d->destFileName = str;
    setText(2, d->destFileName);
}

QString QueueListViewItem::destFileName() const
{
    return d->destFileName;
}

QString QueueListViewItem::destBaseName() const
{
    QFileInfo fi(d->destFileName);

    return fi.completeBaseName();
}

QString QueueListViewItem::destSuffix() const
{
    QFileInfo fi(d->destFileName);

    return fi.suffix();
}

// ---------------------------------------------------------------------------

class Q_DECL_HIDDEN QueueListView::Private
{

public:

    enum RemoveItemsType
    {
        ItemsSelected = 0,
        ItemsDone,
        ItemsAll
    };

public:

    Private() = default;

public:

    bool                        showTips        = false;

    const int                   iconSize        = 64;

    QTimer*                     toolTipTimer    = nullptr;
    QTimer*                     progressTimer   = nullptr;

    ThumbnailLoadThread*        thumbLoadThread = ThumbnailLoadThread::defaultThread();

    QueueSettings               settings;

    AssignedBatchTools          toolsList;

    QueueToolTip*               toolTip         = nullptr;

    QueueListViewItem*          toolTipItem     = nullptr;

    DWorkingPixmap*             progressPix     = nullptr;
};

QueueListView::QueueListView(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    setIconSize(QSize(d->iconSize, d->iconSize));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWhatsThis(i18nc("@info", "This is the list of images to batch process."));

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    viewport()->setAcceptDrops(true);
    viewport()->setMouseTracking(true);

    setColumnCount(3);
    setSortingEnabled(true);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    sortByColumn(1, Qt::AscendingOrder);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList titles;
    titles.append(i18nc("@title: preview item",       "Thumbnail"));
    titles.append(i18nc("@title: original item name", "Original"));
    titles.append(i18nc("@title: target item name",   "Target"));
    setHeaderLabels(titles);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    header()->setSectionResizeMode(2, QHeaderView::Stretch);

    d->toolTip       = new QueueToolTip(this);
    d->toolTipTimer  = new QTimer(this);
    d->progressTimer = new QTimer(this);
    d->progressPix   = new DWorkingPixmap(this);

    // -----------------------------------------------------------

    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)),
            Qt::QueuedConnection);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

QueueListView::~QueueListView()
{
    delete d->toolTip;
    delete d;
}

QPixmap QueueListView::progressPixmapForIndex(int index) const
{
    if ((index >= 0) && (index < d->progressPix->frameCount()))
    {
        return (d->progressPix->frameAt(index));
    }

    return QPixmap();
}

Qt::DropActions QueueListView::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* QueueListView::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* QueueListView::mimeData(const QList<QTreeWidgetItem*> items) const    // clazy:exclude=function-args-by-ref

#endif

{
    QList<QUrl> urls;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;

    for (QTreeWidgetItem* const itm : std::as_const(items))
    {
        const QueueListViewItem* const vitem = dynamic_cast<QueueListViewItem*>(itm);

        if (vitem)
        {
            urls.append(vitem->info().fileUrl());
            albumIDs.append(vitem->info().albumId());
            imageIDs.append(vitem->info().id());
        }
    }

    DItemDrag* const mimeData = new DItemDrag(urls, albumIDs, imageIDs);

    return mimeData;
}

void QueueListView::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QPixmap icon(QIcon::fromTheme(QLatin1String("image-jpeg")).pixmap(48));
    int w   = icon.width();
    int h   = icon.height();

    QPixmap pix(w + 4, h + 4);
    QString text(QString::number(items.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width() - 1, pix.height() - 1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2, 2, w, h, Qt::AlignLeft | Qt::AlignTop, text);
    r.setWidth(qMax(r.width(), r.height()));
    r.setHeight(qMax(r.width(), r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(mimeData(items));
    drag->setPixmap(pix);
    drag->exec();
}

void QueueListView::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void QueueListView::dragMoveEvent(QDragMoveEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;

    if      (
             DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs) ||
             DAlbumDrag::decode(e->mimeData(), urls, albumID)           ||
             DTagListDrag::canDecode(e->mimeData())
            )
    {
        if (DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
        {
            ItemInfoList imageInfoList;

            for (QList<qlonglong>::const_iterator it = imageIDs.constBegin() ;
                 it != imageIDs.constEnd() ; ++it)
            {
                ItemInfo info(*it);

                if (!findItemByInfo(info))
                {
                    imageInfoList.append(info);
                }
            }

            if (!imageInfoList.isEmpty())
            {
                QTreeWidget::dragMoveEvent(e);
                e->accept();

                return;
            }
        }
    }
    else if (e->mimeData()->formats().contains(QLatin1String("digikam/workflow")))
    {
        QTreeWidget::dragMoveEvent(e);
        e->accept();

        return;
    }

    e->ignore();
}

void QueueListView::dropEvent(QDropEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;

    if      (DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
    {
        ItemInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = imageIDs.constBegin() ;
             it != imageIDs.constEnd() ; ++it)
        {
            ItemInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        if (!imageInfoList.isEmpty())
        {
            slotAddItems(imageInfoList);
            e->acceptProposedAction();

            QueueListView* const vitem = dynamic_cast<QueueListView*>(e->source());

            if (vitem && (vitem != this))
            {
                for (const ItemInfo& info : std::as_const(imageInfoList))
                {
                    vitem->removeItemByInfo(info);
                }
            }
        }
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInAlbum(albumID);
        ItemInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin() ;
             it != itemIDs.constEnd() ; ++it)
        {
            ItemInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        if (!imageInfoList.isEmpty())
        {
            slotAddItems(imageInfoList);
            e->acceptProposedAction();
        }
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInTag(tagIDs.first(), true);
        ItemInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin() ;
             it != itemIDs.constEnd() ; ++it)
        {
            ItemInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        if (!imageInfoList.isEmpty())
        {
            slotAddItems(imageInfoList);
            e->acceptProposedAction();
        }
    }
    else if (e->mimeData()->formats().contains(QLatin1String("digikam/workflow")))
    {
        QByteArray ba = e->mimeData()->data(QLatin1String("digikam/workflow"));

        if (ba.size())
        {
            QDataStream ds(ba);
            QString     title;
            ds >> title;
            QueueMgrWindow::queueManagerWindow()->slotAssignQueueSettings(title);

            e->acceptProposedAction();
        }
    }
    else
    {
        e->ignore();
    }

    Q_EMIT signalQueueContentsChanged();
}

void QueueListView::setEnableToolTips(bool val)
{
    d->showTips = val;

    if (!val)
    {
        hideToolTip();
    }
}

void QueueListView::hideToolTip()
{
    d->toolTipItem = nullptr;
    d->toolTipTimer->stop();
    slotToolTip();
}

bool QueueListView::acceptToolTip(const QPoint& pos) const
{
    if (columnAt(pos.x()) == 0)
    {
        return true;
    }

    return false;
}

void QueueListView::slotToolTip()
{
    d->toolTip->setQueueItem(d->toolTipItem);
}

void QueueListView::mouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::NoButton)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(itemAt(e->pos()));

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

        return;
    }

    hideToolTip();
    QTreeWidget::mouseMoveEvent(e);
}

void QueueListView::wheelEvent(QWheelEvent* e)
{
    hideToolTip();
    QTreeWidget::wheelEvent(e);
}

void QueueListView::keyPressEvent(QKeyEvent* e)
{
    hideToolTip();
    QTreeWidget::keyPressEvent(e);
}

void QueueListView::focusOutEvent(QFocusEvent* e)
{
    hideToolTip();
    QTreeWidget::focusOutEvent(e);
}

void QueueListView::leaveEvent(QEvent* e)
{
    hideToolTip();
    QTreeWidget::leaveEvent(e);
}

void QueueListView::slotAddItems(const ItemInfoList& list)
{
    if (list.count() == 0)
    {
        return;
    }

    for (ItemInfoList::ConstIterator it = list.begin() ; it != list.end() ; ++it)
    {
        ItemInfo info                 = *it;

        // Check if the new item already exist in the list.

        bool find                     = false;
        const QueueListViewItem* item = nullptr;

        QTreeWidgetItemIterator iter(this);

        while (*iter)
        {
            item = dynamic_cast<QueueListViewItem*>(*iter);

            if (item && (item->info() == info))
            {
                find = true;
            }

            ++iter;
        }

        if (!find)
        {
            new QueueListViewItem(this, info);
        }
    }

    updateDestFileNames();

    Q_EMIT signalQueueContentsChanged();
}

void QueueListView::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    const QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        ItemInfo info = item->info();
        d->thumbLoadThread->find(ThumbnailIdentifier(info.fileUrl().toLocalFile()));
    }

    QTreeWidget::drawRow(p, opt, index);
}

void QueueListView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && (item->info().fileUrl() == QUrl::fromLocalFile(desc.filePath)))
        {
            if (pix.isNull())
            {
                item->setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(d->iconSize, QIcon::Disabled));
            }
            else
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }

            return;
        }

        ++it;
    }
}

void QueueListView::slotClearList()
{
    removeItems(Private::ItemsAll);

    Q_EMIT signalQueueContentsChanged();
}

void QueueListView::slotRemoveSelectedItems()
{
    removeItems(Private::ItemsSelected);

    Q_EMIT signalQueueContentsChanged();
}

void QueueListView::slotRemoveItemsDone()
{
    removeItems(Private::ItemsDone);

    Q_EMIT signalQueueContentsChanged();
}

void QueueListView::removeItems(int removeType)
{
    hideToolTip();

    bool find;

    do
    {
        find = false;
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

            if (item)
            {
                switch (removeType)
                {
                    case Private::ItemsSelected:
                    {
                        if (item->isSelected())
                        {
                            delete item;
                            find = true;
                        }

                        break;
                    }

                    case Private::ItemsDone:
                    {
                        if (item->isDone())
                        {
                            delete item;
                            find = true;
                        }

                        break;
                    }

                    default:  // Private::ItemsAll
                    {
                        delete item;
                        find = true;

                        break;
                    }
                }
            }

            ++it;
        }
    }
    while (find);

    Q_EMIT signalQueueContentsChanged();
}

void QueueListView::removeItemByInfo(const ItemInfo& info)
{
    removeItemById(info.id());
}

void QueueListView::removeItemById(qlonglong id)
{
    hideToolTip();

    bool find;

    do
    {
        find = false;
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

            if (item && (item->info().id() == id))
            {
                delete item;
                find = true;

                break;
            }

            ++it;
        }
    }
    while (find);

    Q_EMIT signalQueueContentsChanged();
}

bool QueueListView::findItemByInfo(const ItemInfo& info)
{
    return (findItemById(info.id()) ? true : false);
}

QueueListViewItem* QueueListView::findItemById(qlonglong id)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && (item->info().id() == id))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

QueueListViewItem* QueueListView::findItemByUrl(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && (item->info().fileUrl() == url))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

int QueueListView::itemsCount()
{
    int count = 0;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        const QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            ++count;
        }

        ++it;
    }

    return count;
}

void QueueListView::cancelItems()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && item->isBusy())
        {
            item->setCanceled();
        }

        ++it;
    }
}

ItemInfoList QueueListView::itemsList(ItemListType type)
{
    ItemInfoList list;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            switch (type)
            {
                case Pending:
                {
                    if (!item->isDone())
                    {
                        list.append(item->info());
                    }

                    break;
                }

                case Selected:
                {
                    if (item->isSelected())
                    {
                        list.append(item->info());
                    }

                    break;
                }

                default:    // All
                {
                    list.append(item->info());

                    break;
                }
            }
        }

        ++it;
    }

    return list;
}

int QueueListView::pendingItemsCount()
{
    return itemsList(Pending).count();
}

int QueueListView::pendingTasksCount()
{
    int count = 0;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        const QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && !item->isDone())
        {
            count += assignedTools().m_toolsList.count();
        }

        ++it;
    }

    return count;
}

void QueueListView::setSettings(const QueueSettings& _settings)
{
    d->settings = _settings;
    resetQueue();
    updateDestFileNames();
}

QueueSettings QueueListView::settings() const
{
    return d->settings;
}

void QueueListView::setAssignedTools(const AssignedBatchTools& tools)
{
    d->toolsList = tools;
    resetQueue();
    updateDestFileNames();
}

AssignedBatchTools QueueListView::assignedTools() const
{
    return d->toolsList;
}

void QueueListView::slotAssignedToolsChanged(const AssignedBatchTools& tools)
{
    setAssignedTools(tools);
}

void QueueListView::resetQueue()
{
    //reset all items

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            item->reset();
        }

        ++it;
    }
}

void QueueListView::updateDestFileNames()
{
    QMap<QString, QString> renamingResults;

    if (settings().renamingRule == QueueSettings::CUSTOMIZE)
    {
        AdvancedRenameManager manager;

        ParseSettings psettings;
        psettings.parseString = settings().renamingParser;

        QList<ParseSettings> files;
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            const QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

            if (item)
            {
                // Update base name using queue renaming rules.

                ItemInfo info = item->info();
                QFileInfo fi(info.filePath());

                ParseSettings ps;
                ps.fileUrl = QUrl::fromLocalFile(fi.absoluteFilePath());
                files << ps;
            }

            ++it;
        }

        manager.addFiles(files);
        manager.parseFiles(psettings);
        renamingResults = manager.newFileList(true);
    }

    AssignedBatchTools tools = assignedTools();

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            // Update base name using queue renaming rules.

            ItemInfo info     = item->info();
            QFileInfo fi(info.filePath());

            // Update suffix using assigned batch tool rules.

            bool extensionSet = false;
            tools.m_itemUrl   = item->info().fileUrl();
            QString newSuffix = tools.targetSuffix(&extensionSet);
            QString newName   = QString::fromUtf8("%1.%2").arg(fi.completeBaseName()).arg(newSuffix);

            if ((settings().renamingRule == QueueSettings::CUSTOMIZE) && !renamingResults.isEmpty())
            {
                QFileInfo fi2(renamingResults[fi.absoluteFilePath()]);

                if (extensionSet)
                {
                    newName = QString::fromUtf8("%1.%2").arg(fi2.completeBaseName())
                              .arg(newSuffix);
                }
                else
                {
                    newName = fi2.fileName();
                }
            }

            item->setDestFileName(newName);
        }

        ++it;
    }
}

void QueueListView::slotContextMenu()
{
    if (!viewport()->isEnabled())
    {
        return;
    }

    KActionCollection* const acol = QueueMgrWindow::queueManagerWindow()->actionCollection();
    QMenu popmenu(this);
    popmenu.addAction(acol->action(QLatin1String("queuemgr_removeitemssel")));
    popmenu.addSeparator();
    popmenu.addAction(acol->action(QLatin1String("queuemgr_clearlist")));
    popmenu.exec(QCursor::pos());
}

void QueueListView::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (QueueMgrWindow::queueManagerWindow()->isBusy())
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Deleted:
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
        {
            const auto ids = changeset.ids();

            for (const qlonglong& id : ids)
            {
                removeItemById(id);
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

void QueueListView::reloadThumbs(const QUrl& url)
{
    d->thumbLoadThread->find(ThumbnailIdentifier(url.toLocalFile()));
}

void QueueListView::setItemBusy(qlonglong id)
{
    QueueListViewItem* const item = findItemById(id);

    if (item)
    {
        item->setBusy();
        d->progressTimer->start(300);
    }
}

void QueueListView::slotProgressTimerDone()
{
    QTreeWidgetItemIterator it(this);
    int active = 0;

    while (*it)
    {
        QueueListViewItem* const item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && item->isBusy())
        {
            item->animProgress();
            active++;
        }

        ++it;
    }

    if (!active)
    {
        d->progressTimer->stop();
    }
}

} // namespace Digikam

#include "moc_queuelist.cpp"
