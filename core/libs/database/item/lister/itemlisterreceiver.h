/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ItemLister
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "itemlisterrecord.h"
#include "dbjob.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemListerReceiver
{
public:

    ItemListerReceiver()                                 = default;
    virtual ~ItemListerReceiver()                        = default;

    virtual void receive(const ItemListerRecord& record) = 0;
    virtual void error(const QString& /*errMsg*/) {};

private:

    Q_DISABLE_COPY(ItemListerReceiver)
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemListerValueListReceiver : public ItemListerReceiver
{
public:

    ItemListerValueListReceiver() = default;

    void receive(const ItemListerRecord& record) override;
    void error(const QString& errMsg)            override;

public:

    QList<ItemListerRecord> records;
    bool                    hasError = false;

private:

    Q_DISABLE_COPY(ItemListerValueListReceiver)
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemListerJobReceiver : public ItemListerValueListReceiver
{
public:

    explicit ItemListerJobReceiver(DBJob* const job);

    void error(const QString& errMsg) override;
    void sendData();

protected:

    DBJob* const m_job = nullptr;

private:

    Q_DISABLE_COPY(ItemListerJobReceiver)
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemListerJobPartsSendingReceiver : public ItemListerJobReceiver
{
public:

    explicit ItemListerJobPartsSendingReceiver(DBJob* const job, int limit);

    void receive(const ItemListerRecord &record) override;

protected:

    int m_limit = 0;
    int m_count = 0;

private:

    Q_DISABLE_COPY(ItemListerJobPartsSendingReceiver)
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemListerJobGrowingPartsSendingReceiver : public ItemListerJobPartsSendingReceiver
{
public:

    explicit ItemListerJobGrowingPartsSendingReceiver(DBJob* const job,
                                                      int start,
                                                      int end,
                                                      int increment);

    void receive(const ItemListerRecord& record) override;

protected:

    int m_maxLimit  = 0;
    int m_increment = 0;

private:

    Q_DISABLE_COPY(ItemListerJobGrowingPartsSendingReceiver)
};

} // namespace Digikam
