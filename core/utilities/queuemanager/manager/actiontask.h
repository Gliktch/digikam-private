/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions task.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Pankaj Kumar <me at panks dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "actiondata.h"
#include "queuesettings.h"
#include "batchtoolutils.h"
#include "actionthreadbase.h"

namespace Digikam
{

class ActionTask : public ActionJob
{
    Q_OBJECT

public:

    ActionTask();
    ~ActionTask()     override;

    void setSettings(const QueueSettings& settings);
    void setItem(const AssignedBatchTools& tools);

Q_SIGNALS:

    void signalStarting(const Digikam::ActionData& ad);
    void signalFinished(const Digikam::ActionData& ad);

public Q_SLOTS:

    void slotCancel();

protected:

    void run()  override;

private:

    void removeTempFiles(const QList<QUrl>& tmpList);
    void emitActionData(ActionData::ActionStatus st,
                        const QString& mess = QString(),
                        const QUrl& dest = QUrl(),
                        bool noWrite = false);

private:

    /// @note disabled
    explicit ActionTask(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
