/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : List-view for the Showfoto folder view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// QT includes

#include <QTreeView>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QModelIndex>

// Local includes

#include "showfotofolderviewsidebar.h"

namespace ShowFoto
{

class ShowfotoFolderViewBar;

class ShowfotoFolderViewList : public QTreeView
{
    Q_OBJECT

public:

    enum FolderViewMode
    {
        ShortView = 0,
        DetailledView
    };

    enum FolderViewRole
    {
        FileName = 0,
        FileSize,
        FileType,
        FileDate            ///< Modifier date
    };

public:

    explicit ShowfotoFolderViewList(ShowfotoFolderViewSideBar* const view,
                                    ShowfotoFolderViewBar* const bar);
    ~ShowfotoFolderViewList()                                           override;

public Q_SLOTS:

    void slotIconSizeChanged(int);

Q_SIGNALS:

    void signalAddBookmark();

private:

    void contextMenuEvent(QContextMenuEvent*)                           override;
    void mouseMoveEvent(QMouseEvent*)                                   override;
    void wheelEvent(QWheelEvent*)                                       override;
    void keyPressEvent(QKeyEvent*)                                      override;
    void focusOutEvent(QFocusEvent*)                                    override;
    void leaveEvent(QEvent*)                                            override;

    void hideToolTip();
    bool acceptToolTip(const QModelIndex& index)                 const;

private Q_SLOTS:

    void slotToolTip();
    void slotOpenInFileManager();
    void slotActivated(const QModelIndex& index);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
