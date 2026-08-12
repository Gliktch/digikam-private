/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : new items finder.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QStringList>

// Local includes

#include "maintenancetool.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT NewItemsFinder : public MaintenanceTool
{
    Q_OBJECT

public:

    enum FinderMode
    {
        CompleteCollectionScan,   ///< Scan whole collection immediately.
        ScanDeferredFiles,        ///< Defer whole collection scan.
        ScheduleCollectionScan    ///< Scan immediately folders list passed in constructor.
    };

public:

    explicit NewItemsFinder(const FinderMode mode = CompleteCollectionScan,
                            const QStringList& foldersToScan = QStringList(),
                            ProgressItem* const parent = nullptr);
    ~NewItemsFinder()                           override;

private:

    void connectToScanController();

private Q_SLOTS:

    void slotStart()                            override;
    void slotCancel()                           override;
    void slotDone()                             override;

    void slotScanStarted(const QString&);
    void slotPartialScanDone(const QString&);
    void slotTotalFilesToScan(int);
    void slotStartScanningAlbum(const QString& albumRoot, const QString& album);
    void slotFilesScanned(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
