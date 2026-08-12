/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumdragdrop.h"

// Qt includes

#include <QDropEvent>
#include <QMenu>
#include <QIcon>
#include <QTimer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albumpointer.h"
#include "importui.h"
#include "ddragobjects.h"
#include "dio.h"
#include "iteminfo.h"
#include "iteminfolist.h"

namespace Digikam
{

AlbumDragDropHandler::AlbumDragDropHandler(AlbumModel* const model)
    : AlbumModelDragDropHandler(model)
{
}

AlbumModel* AlbumDragDropHandler::model() const
{
    return (static_cast<AlbumModel*>(m_model));
}

bool AlbumDragDropHandler::dropEvent(QAbstractItemView* view,
                                     const QDropEvent* e,
                                     const QModelIndex& droppedOn)
{
    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    if (!view)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error: View is null!";
        return false;
    }

    m_view      = view;
    m_source    = e->source();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_position  = m_view->mapToGlobal(e->position().toPoint());
    m_modifiers = e->modifiers();

#else

    m_position  = m_view->mapToGlobal(e->pos());
    m_modifiers = e->keyboardModifiers();

#endif

    m_destAlbum = model()->palbumForIndex(droppedOn);

    m_imageIDs.clear();
    m_srcURLs.clear();

    if (!m_destAlbum)
    {
        return false;
    }

    if (DAlbumDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        int         albumId = 0;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return false;
        }

        m_droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!m_droppedAlbum)
        {
            return false;
        }

        QTimer::singleShot(0, this, [this]()
            {
                slotMoveCopyAlbum();
                model()->setDropIndex(QModelIndex());
            }
        );

        return true;
    }
    else if (DItemDrag::canDecode(e->mimeData()))
    {

        QList<QUrl>  urls;
        QList<int>   albumIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, albumIDs, m_imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || albumIDs.isEmpty() || m_imageIDs.isEmpty())
        {
            return false;
        }

        QTimer::singleShot(0, this, [this]()
            {
                slotMoveCopyItems();
                model()->setDropIndex(QModelIndex());
            }
        );

        return true;
    }

    // -- DnD from Camera GUI ----------------------------

    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        QTimer::singleShot(0, this, [this]()
            {
                slotCopyFromCamera();
                model()->setDropIndex(QModelIndex());
            }
        );

        return true;
    }

    // -- DnD from an external source ---------------------

    else if (e->mimeData()->hasUrls())
    {
        m_srcURLs = e->mimeData()->urls();

        QTimer::singleShot(0, this, [this]()
            {
                slotMoveCopyExtern();
                model()->setDropIndex(QModelIndex());
            }
        );

        return true;
    }

    return false;
}

Qt::DropAction AlbumDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& dropIndex)
{
    const PAlbum* const destAlbum = model()->palbumForIndex(dropIndex);

    if (!destAlbum)
    {
        return Qt::IgnoreAction;
    }

    // Dropping on root is not allowed and
    // Dropping on trash is not implemented yet

    if (destAlbum->isRoot() || destAlbum->isTrashAlbum())
    {
        return Qt::IgnoreAction;
    }

    if      (DAlbumDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        int         albumId = 0;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return Qt::IgnoreAction;
        }

        const PAlbum* const droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging an item on itself makes no sense

        if (droppedAlbum == destAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging a parent on its child makes no sense

        if (droppedAlbum->isAncestorOf(destAlbum))
        {
            return Qt::IgnoreAction;
        }

        return Qt::MoveAction;
    }
    else if (DItemDrag::canDecode(e->mimeData())           ||
             DCameraItemListDrag::canDecode(e->mimeData()) ||
             e->mimeData()->hasUrls())
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList AlbumDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DAlbumDrag::mimeTypes()
              << DItemDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* AlbumDragDropHandler::createMimeData(const QList<Album*>& albums)
{
    if (albums.isEmpty())
    {
        return nullptr;
    }

    if (albums.size() > 1)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Dragging multiple albums is not implemented";
    }

    const PAlbum* const palbum = dynamic_cast<PAlbum*>(albums.first());

    // Root or album root and Trash Albums are not draggable

    if (!palbum || palbum->isRoot() || palbum->isAlbumRoot() || palbum->isTrashAlbum())
    {
        return nullptr;
    }

    return (new DAlbumDrag(albums.first()->databaseUrl(), albums.first()->id(), palbum->fileUrl()));
}

void AlbumDragDropHandler::slotMoveCopyAlbum()
{
    QMenu popMenu(m_view);
    const QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
    const QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
    popMenu.addSeparator();
    popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

    const QAction* const choice     = popMenu.exec(m_position);

    if      (choice == moveAction)
    {
        DIO::move(m_droppedAlbum, m_destAlbum);
    }
    else if (choice == copyAction)
    {
        DIO::copy(m_droppedAlbum, m_destAlbum);
    }
}

void AlbumDragDropHandler::slotMoveCopyItems()
{
    // Check if items dropped come from outside current album.
    // This can be the case with recursive content album mode.

    ItemInfoList extImgInfList;

    for (QList<qlonglong>::const_iterator it = m_imageIDs.constBegin() ;
         it != m_imageIDs.constEnd(); ++it)
    {
        ItemInfo info(*it);

        if (info.albumId() != m_destAlbum->id())
        {
            extImgInfList << info;
        }
    }

    if (extImgInfList.isEmpty())
    {
        // Setting the dropped image as the album thumbnail
        // If the ctrl key is pressed, when dropping the image, the
        // thumbnail is set without a popup menu

        bool set = false;

        if (m_modifiers == Qt::ControlModifier)
        {
            set = true;
        }
        else
        {
            QMenu popMenu(m_view);
            const QAction* setAction    = nullptr;

            if (m_imageIDs.count() == 1)
            {
                setAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
            }

            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

            const QAction* const choice = popMenu.exec(m_position);
            set                         = (setAction == choice);
        }

        if (set && m_destAlbum)
        {
            QString errMsg;
            AlbumManager::instance()->updatePAlbumIcon(m_destAlbum, m_imageIDs.first(), errMsg);
        }

        return;
    }

    bool ddMove       = false;
    bool ddCopy       = false;
    bool setThumbnail = false;

    if      (m_modifiers == Qt::ShiftModifier)
    {
        // If shift key is pressed while dragging, move the drag object without
        // displaying popup menu -> move

        ddMove = true;
    }
    else if (m_modifiers == Qt::ControlModifier)
    {
        // If ctrl key is pressed while dragging, copy the drag object without
        // displaying popup menu -> copy

        ddCopy = true;
    }
    else
    {
        QMenu popMenu(m_view);
        const QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
        const QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
        const QAction* thumbnailAction  = nullptr;

        if (m_imageIDs.count() == 1)
        {
            thumbnailAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
        }

        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

        const QAction* const choice     = popMenu.exec(m_position);

        if (choice)
        {
            if      (choice == moveAction)
            {
                ddMove = true;
            }
            else if (choice == copyAction)
            {
                ddCopy = true;
            }
            else if (choice == thumbnailAction)
            {
                setThumbnail = true;
            }
        }
    }

    if (!m_destAlbum)
    {
        return;
    }

    if      (ddMove)
    {
        DIO::move(extImgInfList, m_destAlbum);
    }
    else if (ddCopy)
    {
        DIO::copy(extImgInfList, m_destAlbum);
    }
    else if (setThumbnail)
    {
        QString errMsg;
        AlbumManager::instance()->updatePAlbumIcon(m_destAlbum, extImgInfList.first().id(), errMsg);
    }
}

void AlbumDragDropHandler::slotCopyFromCamera()
{
    ImportUI* const ui = dynamic_cast<ImportUI*>(m_source);

    if (ui)
    {
        QMenu popMenu(m_view);
        const QAction* const downAction    = popMenu.addAction(QIcon::fromTheme(QLatin1String("file-export")), i18n("Download From Camera"));
        const QAction* const downDelAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("file-export")), i18n("Download && Delete From Camera"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

        const QAction* const choice        = popMenu.exec(m_position);

        if (choice)
        {
            if      (choice == downAction)
            {
                ui->slotDownload(true, false, m_destAlbum);
            }
            else if (choice == downDelAction)
            {
                ui->slotDownload(true, true, m_destAlbum);
            }
        }
    }
}

void AlbumDragDropHandler::slotMoveCopyExtern()
{
    bool ddMove = false;
    bool ddCopy = false;

    if      (m_modifiers == Qt::ShiftModifier)
    {
        // If shift key is pressed while dropping, move the drag object without
        // displaying popup menu -> move

        ddMove = true;
    }
    else if (m_modifiers == Qt::ControlModifier)
    {
        // If ctrl key is pressed while dropping, copy the drag object without
        // displaying popup menu -> copy

        ddCopy = true;
    }
    else
    {
        QMenu popMenu(m_view);
        const QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
        const QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

        const QAction* const choice     = popMenu.exec(m_position);

        if      (choice == copyAction)
        {
            ddCopy = true;
        }
        else if (choice == moveAction)
        {
            ddMove = true;
        }
    }

    if      (ddMove)
    {
        DIO::move(m_srcURLs, m_destAlbum);
    }
    else if (ddCopy)
    {
        DIO::copy(m_srcURLs, m_destAlbum);
    }
}

} // namespace Digikam

#include "moc_albumdragdrop.cpp"
