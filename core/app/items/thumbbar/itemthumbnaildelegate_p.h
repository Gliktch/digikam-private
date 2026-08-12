/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : thumbnail bar for items - the delegate
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "itemdelegate_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemThumbnailDelegatePrivate : public ItemDelegate::ItemDelegatePrivate
{
public:

    ItemThumbnailDelegatePrivate()
    {
          // switch off drawing of frames

          drawMouseOverFrame  = false;
          drawFocusFrame      = false;

          // switch off composing rating over background

          ratingOverThumbnail = true;
    }

public:

    QListView::Flow flow    = QListView::LeftToRight;
    QRect           viewSize;

public:

    void init(ItemThumbnailDelegate* const q);
};

} // namespace Digikam
