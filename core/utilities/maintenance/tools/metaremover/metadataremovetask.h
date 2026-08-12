/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-21-06
 * Description : Thread actions task for metadata remover.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2023      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "actionthreadbase.h"
#include "metadataremover.h"

namespace Digikam
{

class MaintenanceData;

class MetadataRemoveTask : public ActionJob
{
    Q_OBJECT

public:

    explicit MetadataRemoveTask();
    ~MetadataRemoveTask() override;

    void setRemoveAction(MetadataRemover::RemoveAction action);
    void setMaintenanceData(MaintenanceData* const data = nullptr);

Q_SIGNALS:

    void signalFinished(const ItemInfo&, const QImage&);

protected:

    void run()            override;

private:

    /// @note disabled
    MetadataRemoveTask(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
