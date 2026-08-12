/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Items - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "iteminfo.h"
#include "abstractitemdragdrophandler.h"
#include "itemalbummodel.h"

namespace Digikam
{

class ItemDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ItemDragDropHandler(ItemModel* const model);

    ItemModel*      model()                         const override;
    ItemAlbumModel* albumModel()                    const;

    /**
     * Enables a mode in which dropping will never start an operation
     * which copies or moves files on disk.
     * Only the signals are emitted.
     */
    void setReadOnlyDrop(bool readOnly);

    bool dropEvent(QAbstractItemView* view,
                   const QDropEvent* e,
                   const QModelIndex& droppedOn)          override;

    Qt::DropAction accepts(const QDropEvent* e,
                           const QModelIndex& dropIndex)  override;

    QStringList mimeTypes() const                         override;
    QMimeData* createMimeData(const QList<QModelIndex>&)  override;

Q_SIGNALS:

    void itemInfosDropped(const QList<ItemInfo>& infos);
    void urlsDropped(const QList<QUrl>& urls);
    void assignTags(const QList<ItemInfo>& list, const QList<int>& tagIDs);
    void addToGroup(const ItemInfo& pick, const QList<ItemInfo>& infos);
    void dragDropSort(const ItemInfo& pick, const QList<ItemInfo>& infos);

private:

    enum DropAction
    {
        NoAction,
        CopyAction,
        MoveAction,
        GroupAction,
        SortAction,
        GroupAndMoveAction,
        AssignTagAction
    };

private:

    DropAction copyOrMove(bool allowMove = true,
                          bool askForGrouping = false) const;
    DropAction tagAction(bool askForGrouping)          const;
    DropAction groupAction()                           const;

private Q_SLOTS:

    void slotMoveCopyInternal();
    void slotMoveCopyExternal();
    void slotMoveCopyItemTags();
    void slotDownloadCamItems();

private:

    QAbstractItemView*    m_view       = nullptr;
    QObject*              m_source     = nullptr;
    PAlbum*               m_destPAlbum = nullptr;
    TAlbum*               m_destTAlbum = nullptr;

    Qt::KeyboardModifiers m_modifiers;
    Qt::DropAction        m_proposed   = Qt::IgnoreAction;
    QModelIndex           m_droppedOn;
    QPoint                m_position;

    QList<int>            m_tagIDs;
    QList<QUrl>           m_srcURLs;
    QList<qlonglong>      m_imageIDs;

protected:

    bool                  m_readOnly   = false;
};

} // namespace Digikam
