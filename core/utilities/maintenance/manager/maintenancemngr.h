/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

namespace Digikam
{

class MaintenanceSettings;
class ProgressItem;

class MaintenanceMngr : public QObject
{
    Q_OBJECT

public:

    explicit MaintenanceMngr(QObject* const parent);
    ~MaintenanceMngr() override;

    void setSettings(const MaintenanceSettings& settings);

    void start();
    bool isRunning() const;

Q_SIGNALS:

    void signalComplete();

private Q_SLOTS:

    void slotToolCompleted(ProgressItem*);
    void slotToolCanceled(ProgressItem*);

private:

    // Chained stages

    void stage1();  ///< Find New items
    void stage2();  ///< Database Cleanup
    void stage3();  ///< Update Thumbnails Build
    void stage4();  ///< Similarity Finger-prints
    void stage5();  ///< Find Duplicates
    void stage6();  ///< Faces Management
    void stage7();  ///< Image Quality Sorter
    void stage8();  ///< Autotags Assignment
    void stage9();  ///< Metadata Synchronizer

    void done();    ///< Called when all scheduled tools are done.
    void cancel();  ///< Called when a tool is canceled.

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
