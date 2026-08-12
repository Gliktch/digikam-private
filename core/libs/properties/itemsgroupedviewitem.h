/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-04-11
 * Description : A list view to show grouped items
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QPixmap>
#include <QIcon>

// Local includes

#include "iteminfo.h"

namespace Digikam
{

class ItemsGroupedView;

class ItemsGroupedViewItem : public QTreeWidgetItem
{

public:

    ItemsGroupedViewItem(ItemsGroupedView* const view, const ItemInfo& info);
    ItemsGroupedViewItem(ItemsGroupedViewItem* const item, const ItemInfo& info);
    ~ItemsGroupedViewItem()              override;

    void setInfo(const ItemInfo& info);
    ItemInfo info()             const;

    ItemsGroupedView* view()    const;

    void setThumb(const QPixmap& pix);

    void updateTitle();

private:

    void setPixmap(const QPixmap& pix);

private:

    class Private;
    Private* const d = nullptr;

private:

    Q_DISABLE_COPY(ItemsGroupedViewItem)
};

} // namespace Digikam
