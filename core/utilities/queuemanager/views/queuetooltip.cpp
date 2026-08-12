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

#include "queuetooltip.h"

// Qt includes

#include <QDateTime>
#include <QPainter>
#include <QPixmap>
#include <QTextDocument>

// Local includes

#include "applicationsettings.h"
#include "queuelist.h"
#include "tooltipfiller.h"

namespace Digikam
{

class Q_DECL_HIDDEN QueueToolTip::Private
{
public:

    Private() = default;

    QueueListView*     view = nullptr;
    QueueListViewItem* item = nullptr;
};

QueueToolTip::QueueToolTip(QueueListView* const view)
    : DItemToolTip(),
      d           (new Private)
{
    d->view = view;
}

QueueToolTip::~QueueToolTip()
{
    delete d;
}

void QueueToolTip::setQueueItem(QueueListViewItem* const item)
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

QRect QueueToolTip::repositionRect()
{
    if (!d->item)
    {
        return QRect();
    }

    QRect rect = d->view->visualItemRect(d->item);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString QueueToolTip::tipContents()
{
    if (!d->item)
    {
        return QString();
    }

    ItemInfo info = d->item->info();

    return ToolTipFiller::imageInfoTipContents(info);
}

} // namespace Digikam

#include "moc_queuetooltip.cpp"
