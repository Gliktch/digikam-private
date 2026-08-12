/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-14
 * Description : Table view shared object
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

class QItemSelectionModel;

namespace Digikam
{

class ItemFilterModel;
class ItemModel;
class TableView;
class TableViewColumnFactory;
class TableViewCurrentToSortedSyncer;
class TableViewItemDelegate;
class TableViewModel;
class TableViewSelectionModelSyncer;
class TableViewTreeView;
class ThumbnailLoadThread;

class TableViewShared
{
public:

    TableViewShared()  = default;
    ~TableViewShared() = default;

public:

    ItemModel*                     imageModel                       = nullptr;
    ItemFilterModel*               imageFilterModel                 = nullptr;
    QItemSelectionModel*           imageFilterSelectionModel        = nullptr;
    ThumbnailLoadThread*           thumbnailLoadThread              = nullptr;

    TableView*                     tableView                        = nullptr;
    TableViewTreeView*             treeView                         = nullptr;
    TableViewModel*                tableViewModel                   = nullptr;
    QItemSelectionModel*           tableViewSelectionModel          = nullptr;

    TableViewSelectionModelSyncer* tableViewSelectionModelSyncer    = nullptr;
    TableViewColumnFactory*        columnFactory                    = nullptr;
    TableViewItemDelegate*         itemDelegate                     = nullptr;

    bool                           isActive                         = false;
};

} // namespace Digikam
