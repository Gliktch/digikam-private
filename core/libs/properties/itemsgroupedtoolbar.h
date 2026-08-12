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

#include <QWidget>

namespace Digikam
{

class ItemsGroupedViewItem;
class ItemsGroupedView;

class ItemsGroupedToolBar : public QWidget
{
    Q_OBJECT

public:

    explicit ItemsGroupedToolBar(ItemsGroupedView* const view);

Q_SIGNALS:

    void signalDeleteRequested();
};

} // namespace Digikam
