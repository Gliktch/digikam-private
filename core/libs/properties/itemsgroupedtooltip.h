/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-03
 * Description : queue tool tip
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "iteminfo.h"
#include "ditemtooltip.h"

namespace Digikam
{

class ItemsGroupedView;
class ItemsGroupedViewItem;

class ItemsGroupedToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit ItemsGroupedToolTip(ItemsGroupedView* const view);
    ~ItemsGroupedToolTip()          override;

    void setItem(ItemsGroupedViewItem* const item);

private:

    QRect   repositionRect() override;
    QString tipContents()    override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
