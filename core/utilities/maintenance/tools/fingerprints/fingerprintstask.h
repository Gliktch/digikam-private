/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for finger-prints generator.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>
#include <QThread>

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{

class LoadingDescription;
class MaintenanceData;
class DImg;
class ItemInfo;

class FingerprintsTask : public ActionJob
{
    Q_OBJECT

public:

    explicit FingerprintsTask();
    ~FingerprintsTask() override;

    void setMaintenanceData(MaintenanceData* const data = nullptr);
    void setRebuildAll(bool b);

Q_SIGNALS:

    void signalFinished(const ItemInfo&, const QImage&);

protected:

    void run() override;

private:

    /// @note disabled
    FingerprintsTask(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
