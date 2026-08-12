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

#include "itemsgroupedtooltip.h"

// Qt includes

#include <QDateTime>
#include <QPainter>
#include <QPixmap>
#include <QTextDocument>

// Local includes

#include "applicationsettings.h"
#include "itemsgroupedview.h"
#include "itemsgroupedviewitem.h"
#include "tooltipfiller.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemsGroupedToolTip::Private
{
public:

    Private() = default;

public:

    ItemsGroupedView*     view = nullptr;
    ItemsGroupedViewItem* item = nullptr;
};

ItemsGroupedToolTip::ItemsGroupedToolTip(ItemsGroupedView* const view)
    : DItemToolTip(),
      d           (new Private)
{
    d->view = view;
}

ItemsGroupedToolTip::~ItemsGroupedToolTip()
{
    delete d;
}

void ItemsGroupedToolTip::setItem(ItemsGroupedViewItem* const item)
{
    d->item = item;

    if (!d->item || !ApplicationSettings::instance()->showToolTipsIsValid())
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();

        if (isHidden() && !toolTipIsEmpty())
        {
            show();
        }
    }
}

QRect ItemsGroupedToolTip::repositionRect()
{
    if (!d->item)
    {
        return QRect();
    }

    QRect rect = d->view->visualItemRect(d->item);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString ItemsGroupedToolTip::tipContents()
{
    if (!d->item)
    {
        return QString();
    }

    ItemInfo info = d->item->info();

    return ToolTipFiller::imageInfoTipContents(info);
}

} // namespace Digikam

#include "moc_itemsgroupedtooltip.cpp"
