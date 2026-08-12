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

#pragma once

// QT includes

#include <QList>
#include <QPixmap>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QModelIndex>

// Local includes

#include "showfotostackviewsidebar.h"

namespace ShowFoto
{

class ShowfotoThumbnailBar;

class ShowfotoStackViewList : public QTreeWidget
{
    Q_OBJECT

public:

    enum StackViewRole
    {
        FileName = 0,
        FileSize,
        FileType,
        FileDate            ///< Metadata date if exists, else Modifier date
    };

    enum ThumbnailSize
    {
        SizeSmall  = 32,
        SizeMedium = 48,
        SizeLarge  = 64,
        SizeHuge   = 96
    };

public:

    explicit ShowfotoStackViewList(ShowfotoStackViewSideBar* const view);
    ~ShowfotoStackViewList()                                            override;

    void setThumbbar(ShowfotoThumbnailBar* const thumbbar);

    ShowfotoItemInfo infoFromIndex(const QModelIndex& index)      const;
    QList<QUrl> urls();
    QUrl        currentUrl()                                      const;

    int sortOrder()                                               const;
    int sortRole()                                                const;

public Q_SLOTS:

    void slotIconSizeChanged(int);

Q_SIGNALS:

    void signalItemListChanged(int nbitems);
    void signalAddFavorite();
    void signalClearItemsList();
    void signalRemoveItemInfos(const QList<ShowfotoItemInfo>& infos);
    void signalShowfotoItemInfoActivated(const ShowfotoItemInfo& info);

private:

    void contextMenuEvent(QContextMenuEvent*)                           override;
    void mouseMoveEvent(QMouseEvent*)                                   override;
    void wheelEvent(QWheelEvent*)                                       override;
    void keyPressEvent(QKeyEvent*)                                      override;
    void focusOutEvent(QFocusEvent*)                                    override;
    void leaveEvent(QEvent*)                                            override;

    void hideToolTip();
    bool acceptToolTip(const QModelIndex& index)                  const;

    void drawRow(QPainter* p,
                 const QStyleOptionViewItem& opt,
                 const QModelIndex& index)                        const override;

private Q_SLOTS:

    void slotToolTip();
    void slotOpenInFileManager();
    void slotItemsAdded(const QList<ShowfotoItemInfo>& items);
    void slotItemsRemoved(const QList<ShowfotoItemInfo>& items);
    void slotItemThumbnail(const ShowfotoItemInfo& info, const QPixmap& pix);
    void slotItemsSelected(const QList<ShowfotoItemInfo>& items);
    void slotItemsDeselected(const QList<ShowfotoItemInfo>& items);
    void slotSelectionChanged(QTreeWidgetItem*);
    void slotItemActivated(QTreeWidgetItem*);
    void slotItemsListChanged();
    void slotRemoveItems();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
