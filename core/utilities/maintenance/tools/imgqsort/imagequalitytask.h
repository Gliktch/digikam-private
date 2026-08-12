/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Thread actions task for image quality sorter.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "digikam_globals.h"
#include "actionthreadbase.h"

namespace Digikam
{

class ImageQualitySettings;
class MaintenanceData;
class ItemInfo;

class ImageQualityTask : public ActionJob
{
    Q_OBJECT

public:

    explicit ImageQualityTask();
    ~ImageQualityTask()     override;

    void setQuality(const ImageQualitySettings& quality);
    void setMaintenanceData(MaintenanceData* const data = nullptr);

Q_SIGNALS:

    void signalFinished(const ItemInfo&, const QImage&, int);

public Q_SLOTS:

    void slotCancel();

protected:

    void run()              override;

private:

    /// @note disabled
    ImageQualityTask(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
