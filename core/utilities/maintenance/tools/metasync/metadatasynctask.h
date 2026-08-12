/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions task for metadata synchronizer.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "actionthreadbase.h"
#include "iteminfo.h"
#include "metadatasynchronizer.h"

namespace Digikam
{

class MaintenanceData;

class MetadataSyncTask : public ActionJob
{
    Q_OBJECT

public:

    explicit MetadataSyncTask();
    ~MetadataSyncTask()     override;

    void setTagsOnly(bool value);
    void setDirection(MetadataSynchronizer::SyncDirection dir);
    void setMaintenanceData(MaintenanceData* const data = nullptr);

Q_SIGNALS:

    void signalFinished(const ItemInfo&, const QImage&);
    void signalRemovePending(const ItemInfo& info);

protected:

    void run()              override;

private:

    /// @note disabled
    MetadataSyncTask(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
