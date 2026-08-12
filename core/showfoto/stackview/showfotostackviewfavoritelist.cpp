/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorites list
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewfavoritelist.h"

// Qt includes

#include <QHeaderView>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDrag>
#include <QMenu>
#include <QMimeType>
#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local include

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "showfotostackviewfavorites.h"
#include "showfotostackviewfavoriteitem.h"
#include "dfileoperations.h"
#include "drawdecoder.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoStackViewFavoriteList::Private
{
public:

    Private() = default;

public:

    ShowfotoStackViewFavorites* parent      = nullptr;
    QTreeWidgetItem*            draggedItem = nullptr;
    QString                     filter;
};

ShowfotoStackViewFavoriteList::ShowfotoStackViewFavoriteList(ShowfotoStackViewFavorites* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    d->parent = parent;

    setWhatsThis(i18nc("@info", "You can add or remove favorite stacks here."));
    setAlternatingRowColors(true);
    setColumnCount(1);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    sortByColumn(0, Qt::AscendingOrder);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHeaderHidden(true);
    header()->setSectionResizeMode(QHeaderView::Stretch);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);
    viewport()->setAcceptDrops(true);
    viewport()->setMouseTracking(true);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu(QPoint)));
}

ShowfotoStackViewFavoriteList::~ShowfotoStackViewFavoriteList()
{
    delete d;
}

void ShowfotoStackViewFavoriteList::slotOpenInFileManager()
{
    const ShowfotoStackViewFavoriteItem* const fvitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(currentItem());

    if (fvitem)
    {
        DFileOperations::openInFileManager(fvitem->urls());
    }
}

void ShowfotoStackViewFavoriteList::slotLoadContents()
{
    const ShowfotoStackViewFavoriteItem* const fvitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(currentItem());

    if (fvitem)
    {
        Q_EMIT signalLoadContentsFromFiles(fvitem->urlsToPaths(), fvitem->currentUrl().toLocalFile());
    }
}

void ShowfotoStackViewFavoriteList::slotContextMenu(const QPoint& pos)
{
    QMenu* const ctxmenu                              = new QMenu(this);
    ctxmenu->setTitle(i18nc("@title", "Favorites"));
    ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("AddFavorite")));
    ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("AddFolder")));

    const ShowfotoStackViewFavoriteItem* const fvitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(itemAt(pos));

    if (fvitem)
    {
        if (fvitem->favoriteType() != ShowfotoStackViewFavoriteItem::FavoriteRoot)
        {
            ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("DelItem")));
            ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("EditItem")));
        }

        ctxmenu->addSeparator();

        if (fvitem->favoriteType() == ShowfotoStackViewFavoriteItem::FavoriteItem)
        {
            ctxmenu->addActions(d->parent->pluginActions());
            ctxmenu->addSeparator();

            QAction* const loadContents = new QAction(QIcon::fromTheme(QLatin1String("media-playlist-normal")),
                                                      i18nc("@action: context menu", "Load Contents"), ctxmenu);
            ctxmenu->addAction(loadContents);

            connect(loadContents, SIGNAL(triggered()),
                    this, SLOT(slotLoadContents()));

            QAction* const openFileMngr = new QAction(QIcon::fromTheme(QLatin1String("folder-open")),
                                                      i18nc("@action: context menu", "Open in File Manager"), ctxmenu);
            ctxmenu->addAction(openFileMngr);

            connect(openFileMngr, SIGNAL(triggered()),
                    this, SLOT(slotOpenInFileManager()));
        }
    }

    ctxmenu->exec(mapToGlobal(pos));

    delete ctxmenu;
}

QStringList ShowfotoStackViewFavoriteList::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

Qt::DropActions ShowfotoStackViewFavoriteList::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

void ShowfotoStackViewFavoriteList::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    d->draggedItem    = items.first();

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(mimeData(items));
    drag->exec();
}

void ShowfotoStackViewFavoriteList::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void ShowfotoStackViewFavoriteList::dragMoveEvent(QDragMoveEvent* e)
{
    if ((e->source() == this) &&

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        (dynamic_cast<ShowfotoStackViewFavoriteItem*>(itemAt(e->position().toPoint())) != d->parent->topFavoritesItem()))

#else

        (dynamic_cast<ShowfotoStackViewFavoriteItem*>(itemAt(e->pos())) != d->parent->topFavoritesItem()))

#endif

    {
        QTreeWidget::dragMoveEvent(e);
        e->accept();

        return;
    }

    if (e->mimeData()->hasUrls())
    {
        QList<QUrl> mimeurls = e->mimeData()->urls();
        QList<QUrl> urls;

        for (const QUrl& url : std::as_const(mimeurls))
        {
            if (isReadableImageFile(url.toLocalFile()))
            {
                urls << url;
            }
        }

        if (!urls.isEmpty())
        {
            QTreeWidget::dragMoveEvent(e);
            e->accept();
            return;
        }
    }

    e->ignore();
}

void ShowfotoStackViewFavoriteList::dropEvent(QDropEvent* e)
{
    if (e->source() == this)
    {
        const ShowfotoStackViewFavoriteItem* const src = dynamic_cast<ShowfotoStackViewFavoriteItem*>(d->draggedItem);

        if (!src)
        {
            return;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        ShowfotoStackViewFavoriteItem* const tgt = dynamic_cast<ShowfotoStackViewFavoriteItem*>(itemAt(e->position().toPoint()));

#else

        ShowfotoStackViewFavoriteItem* const tgt = dynamic_cast<ShowfotoStackViewFavoriteItem*>(itemAt(e->pos()));

#endif

        if (!tgt)
        {
            return;
        }

        for (int i = 0 ; i < tgt->childCount() ; ++i)
        {
            const ShowfotoStackViewFavoriteItem* const chl = dynamic_cast<ShowfotoStackViewFavoriteItem*>(tgt->child(i));

            if (chl && (chl->name() == src->name()))
            {
                // Ignore event if a target child item with same name than source already exist.
                return;
            }
        }

        QTreeWidget::dropEvent(e);
        e->acceptProposedAction();

        rebaseHierarchy(tgt);
        d->draggedItem = nullptr;

        return;
    }

    if (e->mimeData()->hasUrls())
    {
        QList<QUrl> mimeurls = e->mimeData()->urls();
        QList<QUrl> urls;

        for (const QUrl& url : std::as_const(mimeurls))
        {
            if (isReadableImageFile(url.toLocalFile()))
            {
                urls << url;
            }
        }

        if (!urls.isEmpty())
        {
            QTreeWidget::dropEvent(e);

            Q_EMIT signalAddFavoriteList(urls, urls.first());

            e->acceptProposedAction();
            return;
        }
    }

    e->ignore();
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* ShowfotoStackViewFavoriteList::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* ShowfotoStackViewFavoriteList::mimeData(const QList<QTreeWidgetItem*> items) const     // clazy:exclude=function-args-by-ref

#endif

{
    QList<QUrl> urls;

    for (QTreeWidgetItem* const itm : std::as_const(items))
    {
        const ShowfotoStackViewFavoriteItem* const vitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(itm);

        if (vitem)
        {
            urls.append(vitem->urls());
        }
    }

    QMimeData* const data = new QMimeData;
    data->setUrls(urls);

    return data;
}


ShowfotoStackViewFavoriteItem* ShowfotoStackViewFavoriteList::findFavoriteByHierarchy(const QString& hierarchy)
{
    ShowfotoStackViewFavoriteItem* item = nullptr;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        item = dynamic_cast<ShowfotoStackViewFavoriteItem*>(*it);

        if (item && (hierarchy == item->hierarchy()))
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

void ShowfotoStackViewFavoriteList::rebaseHierarchy(ShowfotoStackViewFavoriteItem* const parent)
{
    if (parent)
    {
        QTreeWidgetItemIterator it(parent);

        while (*it)
        {
            ShowfotoStackViewFavoriteItem* const item = dynamic_cast<ShowfotoStackViewFavoriteItem*>(*it);

            if (item)
            {
                item->setName(item->name());
            }

            ++it;
        }
    }
}

void ShowfotoStackViewFavoriteList::setFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->filter                            = filter;
    int query                            = 0;
    ShowfotoStackViewFavoriteItem* fitem = nullptr;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        fitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(*it);

        if (fitem)
        {
            if (fitem->name().contains(filter, cs))
            {
                query++;

                QTreeWidgetItem* item = *it;

                do
                {
                    item->setHidden(false);
                    item = item->parent();
                }
                while (item);
            }
            else
            {
                fitem->setHidden(true);
            }
        }

        ++it;
    }

    Q_EMIT signalSearchResult(query);
}

QString ShowfotoStackViewFavoriteList::filter() const
{
    return d->filter;
}

} // namespace ShowFoto

#include "moc_showfotostackviewfavoritelist.cpp"
