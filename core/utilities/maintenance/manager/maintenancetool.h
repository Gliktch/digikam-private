/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : maintenance tool
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "progressmanager.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT MaintenanceTool : public ProgressItem
{
    Q_OBJECT

public:

    explicit MaintenanceTool(const QString& id, ProgressItem* const parent = nullptr);
    ~MaintenanceTool() override;

    /**
     * If true, show a notification message on desktop notification manager
     * with time elapsed to run process.
     */
    void setNotificationEnabled(bool b);

    /**
     * Re-implement this method if your tool is able to use multi-core CPU to process item in parallel
     */
    virtual void setUseMultiCoreCPU(bool) {};

    /**
     * Return progress advance value if the 50ms for the next progress
     * update is completed, otherwise return 0.
     */
    uint checkProgressNeeded() const;

Q_SIGNALS:

    /**
     * Emit when process is done (not canceled).
     */
    void signalComplete();

    /**
     * Emit when process is canceled.
     */
    void signalCanceled();

public Q_SLOTS:

    void start();

protected Q_SLOTS:

    virtual void slotStart();
    virtual void slotDone();
    virtual void slotCancel();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
