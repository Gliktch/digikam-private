/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-11-08
 * Description : face detection, recognition, and training controller
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

#include <QList>
#include <QObject>

// Local includes

#include "maintenancetool.h"
#include "iteminfo.h"
#include "mlpipelinepackagenotify.h"
#include "facescansettings.h"

namespace Digikam
{

class DImg;
class FaceScanSettings;

class FacesEngine : public MaintenanceTool
{
    Q_OBJECT

public:

    enum InputSource
    {
        Albums = 0,
        Infos,
        Ids
    };

public:

    explicit FacesEngine(const FaceScanSettings& settings,
                         ProgressItem* const parent = nullptr);
    ~FacesEngine()                                                  override;

    static QString                          faceScanTaskToString(FaceScanSettings::FaceScanSource source);
    static FaceScanSettings::FaceScanSource faceScanTaskToEnum(const QString& taskName);

Q_SIGNALS:

    void signalScanNotification(const QString& msg, int type);

private Q_SLOTS:

    void slotStart()                                                override;
    void slotUpdateItemCount(const qlonglong itemCount);
    void slotImagesSkipped(const MLPipelinePackageNotify::Ptr&);
    void slotShowOneDetected(const MLPipelinePackageNotify::Ptr&);
    void slotDone()                                                 override;      // cppcheck-suppress virtualCallInConstructor
    void slotCancel()                                               override;      // cppcheck-suppress virtualCallInConstructor

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
