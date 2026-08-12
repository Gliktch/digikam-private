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

#pragma once

// Qt includes

#include <QStringList>

// Local includes

#include "iteminfoset.h"
#include "dbinfoiface.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT BqmInfoIface : public DBInfoIface
{
    Q_OBJECT

public:

    explicit BqmInfoIface(QObject* const parent);
    ~BqmInfoIface()                                          override = default;

public:

    /**
     * Return pending item info list from the current queue.
     */
    QueuePoolItemsList pendingItemInfoListFromCurrentQueue()      const;

    /**
     * Return selected item info list from the current queue.
     */
    QueuePoolItemsList selectedItemInfoListFromCurrentQueue()     const;

    /**
     * Return all item info list from the current queue.
     */
    QueuePoolItemsList allItemInfoListFromCurrentQueue()          const;

private:

    QueuePoolItemsList itemInfoListFromCurrentQueue(int type)     const;
};

} // namespace Digikam
