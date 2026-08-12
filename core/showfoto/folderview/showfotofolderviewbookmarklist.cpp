/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view bookmarks list
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewbookmarklist.h"

// Qt includes

#include <QHeaderView>
#include <QMimeData>
#include <QFileInfo>
#include <QUrl>
#include <QList>
#include <QDir>
#include <QDrag>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local include

#include "digikam_debug.h"
#include "showfotofolderviewbookmarks.h"
#include "showfotofolderviewbookmarkitem.h"
#include "dfileoperations.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewBookmarkList::Private
{
public:

    Private() = default;

public:

    ShowfotoFolderViewBookmarks* parent = nullptr;
};

ShowfotoFolderViewBookmarkList::ShowfotoFolderViewBookmarkList(ShowfotoFolderViewBookmarks* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    d->parent = parent;
    setWhatsThis(i18nc("@info", "You can add or remove bookmarked places here."));
    setAlternatingRowColors(true);
    setColumnCount(1);
    setHeaderHidden(true);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    sortByColumn(0, Qt::AscendingOrder);
    setSelectionMode(QAbstractItemView::SingleSelection);
    header()->setSectionResizeMode(QHeaderView::Stretch);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(true);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    viewport()->setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu(QPoint)));
}

ShowfotoFolderViewBookmarkList::~ShowfotoFolderViewBookmarkList()
{
    delete d;
}

void ShowfotoFolderViewBookmarkList::slotOpenInFileManager()
{
    QList<QUrl> urls;
    const ShowfotoFolderViewBookmarkItem* const fvitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(currentItem());

    if (fvitem)
    {
        urls << QUrl::fromLocalFile(fvitem->path());
    }

    DFileOperations::openInFileManager(urls);
}

void ShowfotoFolderViewBookmarkList::slotLoadContents()
{
    const ShowfotoFolderViewBookmarkItem* const fvitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(currentItem());

    if (fvitem)
    {
        Q_EMIT signalLoadContents(fvitem->path());
    }
}

void ShowfotoFolderViewBookmarkList::slotContextMenu(const QPoint& pos)
{
    ShowfotoFolderViewBookmarkItem* const fvitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(itemAt(pos));

    if (fvitem && (fvitem->parent() == d->parent->topBookmarksItem()))
    {
        QMenu* const ctxmenu        = new QMenu(this);
        ctxmenu->setTitle(i18nc("@title", "Bookmarks"));
        ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("AddBookmark")));
        ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("DelBookmark")));
        ctxmenu->addAction(d->parent->toolBarAction(QLatin1String("EditBookmark")));
        ctxmenu->addSeparator();
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

        ctxmenu->exec(mapToGlobal(pos));

        delete ctxmenu;
    }
}

void ShowfotoFolderViewBookmarkList::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void ShowfotoFolderViewBookmarkList::dragMoveEvent(QDragMoveEvent* e)
{
    if (e->mimeData()->hasUrls())
    {
        QList<QUrl> urls = e->mimeData()->urls();

        QFileInfo inf(urls.first().toLocalFile());

        if (inf.isDir())
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QTreeWidgetItem* const item                  = itemAt(e->position().toPoint());

#else

            QTreeWidgetItem* const item                  = itemAt(e->pos());

#endif

            ShowfotoFolderViewBookmarkItem* const fvitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(item);

            if (
                (item == d->parent->topBookmarksItem()) ||
                (fvitem && (fvitem->parent() == d->parent->topBookmarksItem()))
               )
            {
                QTreeWidget::dragMoveEvent(e);
                e->accept();
                return;
            }
        }
    }

    e->ignore();
}

void ShowfotoFolderViewBookmarkList::dropEvent(QDropEvent* e)
{
    if (e->mimeData()->hasUrls())
    {
        QList<QUrl> urls = e->mimeData()->urls();

        QFileInfo inf(urls.first().toLocalFile());

        if (inf.isDir())
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QTreeWidgetItem* const item                  = itemAt(e->position().toPoint());

#else

            QTreeWidgetItem* const item                  = itemAt(e->pos());

#endif

            ShowfotoFolderViewBookmarkItem* const fvitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(item);

            if (
                (item == d->parent->topBookmarksItem()) ||
                (fvitem && (fvitem->parent() == d->parent->topBookmarksItem()))
               )
            {
                QTreeWidget::dropEvent(e);

                QString path = inf.filePath();

                if (!path.endsWith(QDir::separator()))
                {
                    path.append(QDir::separator());
                }

                Q_EMIT signalAddBookmark(path);

                e->acceptProposedAction();
                return;
            }
        }
    }

    e->ignore();
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* ShowfotoFolderViewBookmarkList::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* ShowfotoFolderViewBookmarkList::mimeData(const QList<QTreeWidgetItem*> items) const     // clazy:exclude=function-args-by-ref

#endif

{
    QList<QUrl> urls;

    for (QTreeWidgetItem* const itm : std::as_const(items))
    {
        const ShowfotoFolderViewBookmarkItem* const vitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(itm);

        if (vitem)
        {
            urls.append(QUrl::fromLocalFile(vitem->path()));
        }
    }

    QMimeData* const data = new QMimeData;
    data->setUrls(urls);

    return data;
}

void ShowfotoFolderViewBookmarkList::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(mimeData(items));
    drag->exec();
}

ShowfotoFolderViewBookmarkItem* ShowfotoFolderViewBookmarkList::bookmarkExists(const QString& path) const
{
    bool found                           = false;
    ShowfotoFolderViewBookmarkItem* item = nullptr;

    for (int i = 0 ; i < d->parent->topBookmarksItem()->childCount() ; ++i)
    {
        item = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(d->parent->topBookmarksItem()->child(i));

        if (item && (path == item->path()))
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        return item;
    }

    return nullptr;
}

QString ShowfotoFolderViewBookmarkList::bookmarkBaseName(const QString& path) const
{
    if (path.endsWith(QDir::separator()))
    {
        return (path.section(QDir::separator(), -2).remove(QDir::separator()));
    }
    else
    {
        return (QFileInfo(path).baseName());
    }
}

} // namespace ShowFoto

#include "moc_showfotofolderviewbookmarklist.cpp"
