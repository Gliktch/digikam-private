/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : thumbnail bar for items - the delegate
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QListView>

// Local includes

#include "itemdelegate.h"

namespace Digikam
{

class ItemCategoryDrawer;
class ItemThumbnailDelegatePrivate;

class ItemThumbnailDelegate : public ItemDelegate
{
    Q_OBJECT

public:

    explicit ItemThumbnailDelegate(ItemCategorizedView* const parent);
    ~ItemThumbnailDelegate()                            override = default;

    void setFlow(QListView::Flow flow);

    /** Returns the minimum or maximum viewport size in the limiting dimension,
     *  width or height, depending on current flow.
     */
    int maximumSize()                             const;
    int minimumSize()                             const;

    void setDefaultViewOptions(const QStyleOptionViewItem& option) override;
    bool acceptsActivation(const QPoint& pos,
                           const QRect& visualRect,
                           const QModelIndex& index,
                           QRect* activationRect) const override;

protected:

    void updateContentWidth()                           override;
    void updateRects()                                  override;

private:

    Q_DECLARE_PRIVATE(ItemThumbnailDelegate)
};

} // namespace Digikam
