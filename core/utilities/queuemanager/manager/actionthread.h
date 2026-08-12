/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Pankaj Kumar <me at panks dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "batchtool.h"
#include "batchtoolutils.h"
#include "actionthreadbase.h"

namespace Digikam
{

class ActionData;
class QueueSettings;

class ActionThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit ActionThread(QObject* const parent);
    ~ActionThread() override;

    void setSettings(const QueueSettings& settings);

    void processQueueItems(const QList<AssignedBatchTools>& items);

    void cancel()   override;

Q_SIGNALS:

    /**
     * Emit when an item from a queue start to be processed.
     */
    void signalStarting(const Digikam::ActionData& ad);

    /**
     * Emit when an item from a queue have been processed.
     */
    void signalFinished(const Digikam::ActionData& ad);

    /**
     * Emit when a queue have been fully processed (all items from queue are finished).
     */
    void signalQueueProcessed();

    /**
     * Signal to emit to sub-tasks to cancel processing.
     */
    void signalCancelActionTask();

private Q_SLOTS:

    void slotUpdateItemInfo(const Digikam::ActionData& ad);
    void slotThreadFinished();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
