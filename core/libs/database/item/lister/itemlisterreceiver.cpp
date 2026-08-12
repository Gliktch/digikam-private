/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ItemLister
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemlisterreceiver.h"

// Qt includes


namespace Digikam
{

void ItemListerValueListReceiver::error(const QString&)
{
    hasError = true;
}

void ItemListerValueListReceiver::receive(const ItemListerRecord& record)
{
    records << record;
}

// ----------------------------------------------

ItemListerJobReceiver::ItemListerJobReceiver(DBJob* const job)
    : m_job(job)
{
}

void ItemListerJobReceiver::sendData()
{
    Q_EMIT m_job->data(records);

    records.clear();
}

void ItemListerJobReceiver::error(const QString& errMsg)
{
    Q_EMIT m_job->error(errMsg);

    ItemListerValueListReceiver::error(errMsg);
}

// ----------------------------------------------

ItemListerJobPartsSendingReceiver::ItemListerJobPartsSendingReceiver(DBJob* const job, int limit)
    : ItemListerJobReceiver(job),
      m_limit              (limit)
{
}

void ItemListerJobPartsSendingReceiver::receive(const ItemListerRecord& record)
{
    ItemListerJobReceiver::receive(record);

    if (++m_count > m_limit)
    {
        sendData();
        m_count = 0;
    }
}

// ----------------------------------------------

ItemListerJobGrowingPartsSendingReceiver::ItemListerJobGrowingPartsSendingReceiver(DBJob* const job,
                                                                                   int start,
                                                                                   int end,
                                                                                   int increment)
    : ItemListerJobPartsSendingReceiver(job, start),
      m_maxLimit                       (end),
      m_increment                      (increment)
{
}

void ItemListerJobGrowingPartsSendingReceiver::receive(const ItemListerRecord& record)
{
    ItemListerJobPartsSendingReceiver::receive(record);

    // limit was reached?

    if (m_count == 0)
    {
        m_limit = qMin(m_limit + m_increment, m_maxLimit);
    }
}

} // namespace Digikam
