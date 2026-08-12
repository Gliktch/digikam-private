/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-11-08
 * Description : Object detection and image auto-tagging engine
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QObject>

// Local includes

#include "maintenancetool.h"
#include "mlpipelinepackagenotify.h"
#include "autotagsscansettings.h"

namespace Digikam
{

class DImg;

class AutotagsEngine : public MaintenanceTool
{
    Q_OBJECT

public:

    explicit AutotagsEngine(const AutotagsScanSettings& _settings,
                            ProgressItem* const parent = nullptr);
    ~AutotagsEngine()                                               override;

Q_SIGNALS:

    void signalScanNotification(const QString& msg, int type);

private Q_SLOTS:

    void slotStart()                                                override;
    void slotUpdateItemCount(const qlonglong itemCount);
    void slotImagesSkipped(const MLPipelinePackageNotify::Ptr&);
    void slotShowOneDetected(const MLPipelinePackageNotify::Ptr&);
    void slotDone()                                                 override;   // cppcheck-suppress virtualCallInConstructor
    void slotCancel()                                               override;   // cppcheck-suppress virtualCallInConstructor

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
