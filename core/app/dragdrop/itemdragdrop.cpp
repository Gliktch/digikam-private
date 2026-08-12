/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Items - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemdragdrop.h"

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
#include "importui.h"
#include "importiconview.h"
#include "itemthumbnailbar.h"
#include "ddragobjects.h"
#include "dio.h"
#include "itemcategorizedview.h"
#include "iteminfolist.h"
#include "tableview_treeview.h"
#include "digikamitemview.h"
#include "facetags.h"

namespace Digikam
{

static QAction* addGroupAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("go-bottom")),
                           i18nc("@action:inmenu Group images with this item",
                                 "Group here"));
}

static QAction* addSortAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("insert-image")),
                           i18nc("@action:inmenu Insert dragged images before this item",
                                 "Insert Items here"));
}

static QAction* addGroupAndMoveAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("go-bottom")),
                           i18nc("@action:inmenu Group images with this item and move them to its album",
                                 "Group here and move to album"));
}

static QAction* addCancelAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
}

// -------------------------------------------------------------------------------------

ItemDragDropHandler::ItemDragDropHandler(ItemModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

ItemModel* ItemDragDropHandler::model() const
{
    return static_cast<ItemModel*>(m_model);
}

ItemAlbumModel* ItemDragDropHandler::albumModel() const
{
    return qobject_cast<ItemAlbumModel*>(model());
}

void ItemDragDropHandler::setReadOnlyDrop(bool readOnly)
{
    m_readOnly = readOnly;
}

bool ItemDragDropHandler::dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn)
{
    m_view = view;

    if (!m_view)
    {
        return false;
    }

    Album* album = nullptr;

    // Note that the drop event does not have to be in an ItemCategorizedView.
    // It can also be a TableViewTreeView.

    const ItemCategorizedView* const itemView = qobject_cast<ItemCategorizedView*>(m_view);

    if (itemView)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        album = itemView->albumAt(e->position().toPoint());

#else

        album = itemView->albumAt(e->pos());

#endif

    }
    else
    {
        const TableViewTreeView* const tableView = qobject_cast<TableViewTreeView*>(m_view);

        if (tableView)
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            album = tableView->albumAt(e->position().toPoint());

#else

            album = tableView->albumAt(e->pos());

#endif

        }
    }

    // unless we are readonly anyway, we always want an album

    if (!m_readOnly && (!album || album->isRoot()))
    {
        return false;
    }

    m_droppedOn = droppedOn;
    m_source    = e->source();
    m_proposed  = e->proposedAction();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    m_position  = m_view->mapToGlobal(e->position().toPoint());
    m_modifiers = e->modifiers();

#else

    m_position  = m_view->mapToGlobal(e->pos());
    m_modifiers = e->keyboardModifiers();

#endif

    m_tagIDs.clear();
    m_srcURLs.clear();
    m_imageIDs.clear();

    m_destPAlbum = nullptr;
    m_destTAlbum = nullptr;

    if (album)
    {
        Album* currentAlbum = nullptr;

        if (albumModel() && !(albumModel()->currentAlbums().isEmpty()))
        {
            currentAlbum = albumModel()->currentAlbums().constFirst();
        }

        if      (album->type() == Album::PHYSICAL)
        {
            m_destPAlbum = static_cast<PAlbum*>(album);
        }
        else if (currentAlbum && (currentAlbum->type() == Album::PHYSICAL))
        {
            m_destPAlbum = static_cast<PAlbum*>(currentAlbum);
        }

        if      (album->type() == Album::TAG)
        {
            m_destTAlbum = static_cast<TAlbum*>(album);
        }
        else if (currentAlbum && (currentAlbum->type() == Album::TAG))
        {
            m_destTAlbum = static_cast<TAlbum*>(currentAlbum);
        }
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        // Drag & drop inside of digiKam

        QList<QUrl> urls;
        QList<int>  albumIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, albumIDs, m_imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || albumIDs.isEmpty() || m_imageIDs.isEmpty())
        {
            return false;
        }

        QTimer::singleShot(0, this, &ItemDragDropHandler::slotMoveCopyInternal);

    }
    else if (e->mimeData()->hasUrls())
    {
        if (!m_destPAlbum && !m_readOnly)
        {
            return false;
        }

        // Drag & drop outside of digiKam

        m_srcURLs = e->mimeData()->urls();

        if (m_readOnly)
        {
            Q_EMIT urlsDropped(m_srcURLs);

            return true;
        }

        QTimer::singleShot(0, this, &ItemDragDropHandler::slotMoveCopyExternal);

        return true;
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        bool isDecoded = DTagListDrag::decode(e->mimeData(), m_tagIDs);

        if (!isDecoded)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Error: Decoding failed!";
            return false;
        }

        if (!itemView)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Error: View is null!";
            return false;
        }

        QTimer::singleShot(0, this, &ItemDragDropHandler::slotMoveCopyItemTags);

        return true;
    }
    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        if (!m_destPAlbum)
        {
            return false;
        }

        QTimer::singleShot(0, this, &ItemDragDropHandler::slotDownloadCamItems);

        return true;
    }

    return false;
}

Qt::DropAction ItemDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (albumModel() && albumModel()->currentAlbums().isEmpty())
    {
        return Qt::IgnoreAction;
    }

    if (DItemDrag::canDecode(e->mimeData()) || e->mimeData()->hasUrls())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if (e->modifiers() & Qt::ControlModifier)

#else

        if (e->keyboardModifiers() & Qt::ControlModifier)

#endif

        {
            return Qt::CopyAction;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        else if (e->modifiers() & Qt::ShiftModifier)

#else

        else if (e->keyboardModifiers() & Qt::ShiftModifier)

#endif

        {
            return Qt::MoveAction;
        }

        return Qt::MoveAction;
    }

    if (
        DTagListDrag::canDecode(e->mimeData())        ||
        DCameraItemListDrag::canDecode(e->mimeData()) ||
        DCameraDragObject::canDecode(e->mimeData())
       )
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList ItemDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DItemDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << DCameraDragObject::mimeTypes()
              << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* ItemDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ItemInfo> infos = model()->imageInfos(indexes);

    QList<QUrl>      urls;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;

    for (const ItemInfo& info : std::as_const(infos))
    {
        urls.append(info.fileUrl());
        albumIDs.append(info.albumId());
        imageIDs.append(info.id());
    }

    if (urls.isEmpty())
    {
        return nullptr;
    }

    return new DItemDrag(urls, albumIDs, imageIDs);
}

ItemDragDropHandler::DropAction ItemDragDropHandler::copyOrMove(bool allowMove, bool askForGrouping) const
{
    if      (m_modifiers & Qt::ControlModifier)
    {
        return CopyAction;
    }
    else if (m_modifiers & Qt::ShiftModifier)
    {
        return MoveAction;
    }

    if (!allowMove && !askForGrouping)
    {
        switch (m_proposed)
        {
            case Qt::CopyAction:
            {
                return CopyAction;
            }

            case Qt::MoveAction:
            {
                return MoveAction;
            }

            default:
            {
                return NoAction;
            }
        }
    }

    QMenu popMenu(m_view);
    const QAction* moveAction       = nullptr;

    if (allowMove)
    {
        moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),
                                       i18n("&Move Here"));
    }

    const QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")),
                                                        i18n("&Copy Here"));
    popMenu.addSeparator();

    const QAction* grpAction        = nullptr;
    const QAction* grpAndMoveAction = nullptr;

    if (askForGrouping)
    {
        grpAction        = addGroupAction(&popMenu);
        grpAndMoveAction = addGroupAndMoveAction(&popMenu);
        popMenu.addSeparator();
    }

    addCancelAction(&popMenu);

    const QAction* const choice       = popMenu.exec(m_position);

    if      (moveAction && (choice == moveAction))
    {
        return MoveAction;
    }
    else if (choice == copyAction)
    {
        return CopyAction;
    }
    else if (grpAction && (choice == grpAction))
    {
        return GroupAction;
    }
    else if (grpAndMoveAction && (choice == grpAndMoveAction))
    {
        return GroupAndMoveAction;
    }

    return NoAction;
}

ItemDragDropHandler::DropAction ItemDragDropHandler::tagAction(bool askForGrouping) const
{
    QMenu popMenu(m_view);
    const QAction* const tagAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                       i18n("Assign Tag to Dropped Items"));
    const QAction* grpAction       = nullptr;

    if (askForGrouping)
    {
        popMenu.addSeparator();
        grpAction = addGroupAction(&popMenu);
    }

    popMenu.addSeparator();
    addCancelAction(&popMenu);

    const QAction* const choice    = popMenu.exec(m_position);

    if      (grpAction && (choice == grpAction))
    {
        return GroupAction;
    }
    else if (tagAction && (choice == tagAction))
    {
        return AssignTagAction;
    }

    return NoAction;
}

ItemDragDropHandler::DropAction ItemDragDropHandler::groupAction() const
{
    const ItemCategorizedView* const imgView = dynamic_cast<ItemCategorizedView*>(m_view);
    int sortOrder                            = ApplicationSettings::instance()->getImageSortOrder();

    QMenu popMenu(m_view);
    const QAction* sortAction                = nullptr;

    if (imgView &&
        (
            (sortOrder == ItemSortSettings::SortByManualOrderAndName) ||
            (sortOrder == ItemSortSettings::SortByManualOrderAndDate)
        )
       )
    {
        sortAction = addSortAction(&popMenu);
        popMenu.addSeparator();
    }

    const QAction* const groupAction = addGroupAction(&popMenu);
    popMenu.addSeparator();
    addCancelAction(&popMenu);

    const QAction* const choice      = popMenu.exec(m_position);

    if (groupAction && (choice == groupAction))
    {
        return GroupAction;
    }

    if (sortAction && (choice == sortAction))
    {
        return SortAction;
    }

    return NoAction;
}

void ItemDragDropHandler::slotMoveCopyInternal()
{
    DropAction action = NoAction;

    ItemInfo droppedOnInfo;

    if (m_droppedOn.isValid())
    {
        const ItemThumbnailBar* const thumbBar = qobject_cast<ItemThumbnailBar*>(m_view);

        if (thumbBar)
        {
            droppedOnInfo = model()->imageInfo(thumbBar->itemFilterModel()->mapToSourceItemModel(m_droppedOn));
        }
        else
        {
            droppedOnInfo = model()->imageInfo(m_droppedOn);
        }
    }

    if      (m_readOnly)
    {
        Q_EMIT itemInfosDropped(ItemInfoList(m_imageIDs));

        return;
    }
    else if (m_destPAlbum)
    {
        // Check if items dropped come from outside current album.

        QList<ItemInfo> extImages, intImages;

        for (QList<qlonglong>::const_iterator it = m_imageIDs.constBegin() ;
             it != m_imageIDs.constEnd() ; ++it)
        {
            ItemInfo info(*it);

            if (info.albumId() != m_destPAlbum->id())
            {
                extImages << info;
            }
            else
            {
                intImages << info;
            }
        }

        bool onlyExternal = (intImages.isEmpty() && !extImages.isEmpty());
        bool onlyInternal = (!intImages.isEmpty() && extImages.isEmpty());
        bool mixed        = (!intImages.isEmpty() && !extImages.isEmpty());

        // Check for drop of image on itself

        if (
            (intImages.size()  == 1) &&
            (intImages.first() == droppedOnInfo)
           )
        {
            return;
        }

        if (
            onlyInternal                 &&
            m_droppedOn.isValid()        &&
            (m_modifiers == Qt::NoModifier)
           )
        {
            action = groupAction();
        }
        else
        {
            // Determine action. Show Menu only if there are any album-external items.
            // Ask for grouping if dropped-on is valid (gives LinkAction)

            action = copyOrMove(mixed || onlyExternal, !droppedOnInfo.isNull());
        }

        if      (onlyExternal)
        {
            // Only external items: copy or move as requested

            if      (action == MoveAction)
            {
                DIO::move(extImages, m_destPAlbum);
                return;
            }
            else if (action == CopyAction)
            {
                DIO::copy(extImages, m_destPAlbum);
                return;
            }
        }
        else if (onlyInternal)
        {
            // Only items from the current album:
            // Move is a no-op. Do not show menu to ask for copy or move.
            // If the user indicates a copy operation (holding Ctrl), copy.

            if      (action == CopyAction)
            {
                DIO::copy(intImages, m_destPAlbum);
                return;
            }
            else if (action == MoveAction)
            {
                return;
            }
        }
        else if (mixed)
        {
            // Mixed items.
            // For move operations, ignore items from current album.
            // If user requests copy, copy.

            if      (action == MoveAction)
            {
                DIO::move(extImages, m_destPAlbum);
                return;
            }
            else if (action == CopyAction)
            {
                DIO::copy(extImages + intImages, m_destPAlbum);
                return;
            }
        }
    }
    else if (m_destTAlbum)
    {
        if (m_destTAlbum->hasProperty(TagPropertyName::person()))
        {
            return;
        }

        action = tagAction(m_droppedOn.isValid());

        if (action == AssignTagAction)
        {
            Q_EMIT assignTags(ItemInfoList(m_imageIDs), QList<int>() << m_destTAlbum->id());

            return;
        }
    }
    else
    {
        if (m_droppedOn.isValid())
        {
            // Ask if the user wants to group

            action = groupAction();
        }
    }

    if (action == GroupAction)
    {
        if (droppedOnInfo.isNull())
        {
            return;
        }

        Q_EMIT addToGroup(droppedOnInfo, ItemInfoList(m_imageIDs));

        return;
    }

    if (action == SortAction)
    {
        if (droppedOnInfo.isNull())
        {
            return;
        }

        Q_EMIT dragDropSort(droppedOnInfo, ItemInfoList(m_imageIDs));

        return;
    }

    if (action == GroupAndMoveAction)
    {
        if (droppedOnInfo.isNull())
        {
            return;
        }

        Q_EMIT addToGroup(droppedOnInfo, ItemInfoList(m_imageIDs));

        DIO::move(ItemInfoList(m_imageIDs), m_destPAlbum);

        return;
    }
}

void ItemDragDropHandler::slotMoveCopyExternal()
{
    DropAction action = copyOrMove();

    if      (action == MoveAction)
    {
        DIO::move(m_srcURLs, m_destPAlbum);
    }
    else if (action == CopyAction)
    {
        DIO::copy(m_srcURLs, m_destPAlbum);
    }
}

void ItemDragDropHandler::slotMoveCopyItemTags()
{
    // Face tags

    if (m_destTAlbum && m_destTAlbum->hasProperty(TagPropertyName::person()))
    {
        if (
            (m_tagIDs.first() == FaceTags::unconfirmedPersonTagId()) ||
            (m_tagIDs.first() == FaceTags::unknownPersonTagId())     ||
            !FaceTags::isPerson(m_tagIDs.first())
           )
        {
            return;
        }

        DigikamItemView* const dview = qobject_cast<DigikamItemView*>(m_view);

        if (!dview || !m_droppedOn.isValid())
        {
            return;
        }

        QMenu popFaceTagMenu(m_view);

        const QAction* assignFace  = nullptr;
        const QAction* assignFaces = nullptr;

        if (dview->selectionModel()->selectedIndexes().size() > 1)
        {
            assignFaces = popFaceTagMenu.addAction(QIcon::fromTheme(QLatin1String("tag")), i18n("Change Face Tags"));
        }
        else
        {
            assignFace = popFaceTagMenu.addAction(QIcon::fromTheme(QLatin1String("tag")), i18n("Change Face Tag"));
        }

        popFaceTagMenu.addSeparator();
        popFaceTagMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("&Cancel"));

        const QAction* const res = popFaceTagMenu.exec(m_position);

        if (res)
        {
            if      (res == assignFace)
            {
                dview->confirmFaces({m_droppedOn}, m_tagIDs.first());
            }
            else if (res == assignFaces)
            {
                dview->confirmFaces(dview->selectionModel()->selectedIndexes(), m_tagIDs.first());
            }
        }

        return;
    }

    // Standard tags

    const ItemCategorizedView* const itemView = qobject_cast<ItemCategorizedView*>(m_view);

    if (!itemView)
    {
        return;
    }

    QMenu popMenu(m_view);

    QList<ItemInfo> selectedInfos         = itemView->selectedItemInfosCurrentFirst();
    const QAction* assignToSelectedAction = nullptr;

    if (selectedInfos.count() > 1)
    {
        assignToSelectedAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                   i18n("Assign Tags to &Selected Items"));
    }

    const QAction* assignToThisAction = nullptr;

    if (m_droppedOn.isValid())
    {
        assignToThisAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                               i18n("Assign Tags to &This Item"));
    }

    const QAction* const assignToAllAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                               i18n("Assign Tags to &All Items"));

    popMenu.addSeparator();
    popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("&Cancel"));

    const QAction* const choice = popMenu.exec(m_position);

    if (choice)
    {
        if      (choice == assignToSelectedAction)    // Selected Items
        {
            Q_EMIT assignTags(selectedInfos, m_tagIDs);
        }
        else if (choice == assignToAllAction)    // All Items
        {
            Q_EMIT assignTags(itemView->allItemInfos(), m_tagIDs);
        }
        else if (choice == assignToThisAction)    // Dropped item only.
        {
            QModelIndex dropIndex                  = m_droppedOn;
            const ItemThumbnailBar* const thumbBar = qobject_cast<ItemThumbnailBar*>(m_view);

            if (thumbBar)
            {
                dropIndex = thumbBar->itemFilterModel()->mapToSourceItemModel(m_droppedOn);
            }

            Q_EMIT assignTags(QList<ItemInfo>() << model()->imageInfo(dropIndex), m_tagIDs);
        }
    }
}

void ItemDragDropHandler::slotDownloadCamItems()
{
    const ImportIconView* const iconView = dynamic_cast<ImportIconView*>(m_source);

    if (!iconView)
    {
        return;
    }

    QMenu popMenu(m_view);
    popMenu.addSection(QIcon::fromTheme(QLatin1String("digikam")), i18n("Importing"));
    const QAction* const downAction    = popMenu.addAction(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")),
                                                           i18n("Download From Camera"));
    const QAction* const downDelAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")),
                                                           i18n("Download && Delete From Camera"));
    popMenu.addSeparator();
    popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));

    const QAction* const choice        = popMenu.exec(m_position);

    if (choice)
    {
        if      (choice == downAction)
        {
            ImportUI::instance()->slotDownload(true, false, m_destPAlbum);
        }
        else if (choice == downDelAction)
        {
            ImportUI::instance()->slotDownload(true, true, m_destPAlbum);
        }
    }
}

} // namespace Digikam

#include "moc_itemdragdrop.cpp"
