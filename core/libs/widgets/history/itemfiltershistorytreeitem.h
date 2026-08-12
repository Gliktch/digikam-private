/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : widget for displaying an item in view with used filters on current image
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ItemFiltersHistoryTreeItem
{
public:

    explicit ItemFiltersHistoryTreeItem(const QList<QVariant>& data,
                                        ItemFiltersHistoryTreeItem* const parent = nullptr);
    explicit ItemFiltersHistoryTreeItem(const QString& data,
                                        ItemFiltersHistoryTreeItem* const parent = nullptr);
    ~ItemFiltersHistoryTreeItem();

    void appendChild(ItemFiltersHistoryTreeItem* const child);
    void removeChild(int row);

    ItemFiltersHistoryTreeItem* child(int row)  const;
    int childCount()                            const;
    int columnCount()                           const;
    QVariant data(int column)                   const;
    int row()                                   const;
    ItemFiltersHistoryTreeItem* parent()        const;
    void setDisabled(bool disabled)             const;
    bool isDisabled()                           const;

private:

    /// @note disabled
    ItemFiltersHistoryTreeItem(const ItemFiltersHistoryTreeItem&);
    ItemFiltersHistoryTreeItem& operator=(const ItemFiltersHistoryTreeItem&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
