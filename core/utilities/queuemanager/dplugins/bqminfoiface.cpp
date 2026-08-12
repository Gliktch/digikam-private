/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to database information for BQM tools.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bqminfoiface.h"

// Local includes

#include "queuemgrwindow.h"
#include "queuepool.h"
#include "queuelist.h"

namespace Digikam
{

BqmInfoIface::BqmInfoIface(QObject* const parent)
    : DBInfoIface(parent)
{
    setObjectName(QLatin1String("BqmInfoIface"));
}

QueuePoolItemsList BqmInfoIface::pendingItemInfoListFromCurrentQueue() const
{
    return itemInfoListFromCurrentQueue(QueueListView::Pending);
}

QueuePoolItemsList BqmInfoIface::selectedItemInfoListFromCurrentQueue() const
{
    return itemInfoListFromCurrentQueue(QueueListView::Selected);
}

QueuePoolItemsList BqmInfoIface::allItemInfoListFromCurrentQueue() const
{
    return itemInfoListFromCurrentQueue(QueueListView::All);
}

QueuePoolItemsList BqmInfoIface::itemInfoListFromCurrentQueue(int type) const
{
    QueuePool* const pool   = QueueMgrWindow::queueManagerWindow()->queuePool();
    QueuePoolItemsList list = pool->itemsList(pool->currentIndex(), type);

    return list;
}

} // namespace Digikam

#include "moc_bqminfoiface.cpp"
