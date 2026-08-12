/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "itemdelegate.h"

namespace Digikam
{

class ItemCategoryDrawer;
class DigikamItemDelegatePrivate;

class DigikamItemDelegate : public ItemDelegate
{
    Q_OBJECT

public:

    explicit DigikamItemDelegate(ItemCategorizedView* const parent);
    ~DigikamItemDelegate() override = default;

protected:

    void updateRects() override;

    DigikamItemDelegate(DigikamItemDelegatePrivate& dd, ItemCategorizedView* parent);

private:

    /// @note disabled
    DigikamItemDelegate(QObject*) = delete;

private:

    Q_DECLARE_PRIVATE(DigikamItemDelegate)
};

} // namespace Digikam
