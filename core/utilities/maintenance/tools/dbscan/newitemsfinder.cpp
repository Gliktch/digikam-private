/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : new items finder.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "newitemsfinder.h"

// Qt includes

#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "scancontroller.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class Q_DECL_HIDDEN NewItemsFinder::Private
{
public:

    Private() = default;

public:

    FinderMode  mode   = CompleteCollectionScan;

    bool        cancel = false;

    QStringList foldersToScan;
    QStringList foldersScanned;
};

NewItemsFinder::NewItemsFinder(const FinderMode mode,
                               const QStringList& foldersToScan,
                               ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("NewItemsFinder"), parent),
      d              (new Private)
{
    d->mode          = mode;
    d->foldersToScan = foldersToScan;
    d->foldersToScan.sort();
}

NewItemsFinder::~NewItemsFinder()
{
    delete d;
}

void NewItemsFinder::slotStart()
{
    MaintenanceTool::slotStart();

    setShowAtStart(true);
    setLabel(i18n("Find new items"));
    setThumbnail(QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(48));

    switch (d->mode)
    {
        case ScanDeferredFiles:
        {
            connectToScanController();

            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "scan mode: ScanDeferredFiles";

            ScanController::instance()->completeCollectionScanInBackground(false);
            ScanController::instance()->allowToScanDeferredFiles();
            break;
        }

        case CompleteCollectionScan:
        {
            connectToScanController();

            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "scan mode: CompleteCollectionScan";

            ScanController::instance()->completeCollectionScanInBackground(false);

            if (d->cancel)
            {
                break;
            }

            ScanController::instance()->allowToScanDeferredFiles();
            ScanController::instance()->completeCollectionScanInBackground(true);
            break;
        }

        case ScheduleCollectionScan:
        {
            connectToScanController();

            d->foldersScanned.clear();

            QStringList folderList;

            // We scan recursively. Remove all folders
            // that are recursively contained in the list.

            for (const QString& src : std::as_const(d->foldersToScan))
            {
                CollectionLocation location = CollectionManager::instance()->locationForPath(src);

                if (!location.isAvailable())
                {
                    continue;
                }

                QString srcFolder = QDir::cleanPath(src) + QLatin1Char('/');
                bool foundFolder  = false;

                for (const QString& dst : std::as_const(folderList))
                {
                    QString dstFolder = dst + QLatin1Char('/');

                    if (
                         dstFolder.startsWith(srcFolder) ||
                         srcFolder.startsWith(dstFolder)
                       )
                    {
                        foundFolder = true;
                        break;
                    }
                }

                if (!foundFolder)
                {
                    folderList << srcFolder.chopped(1);
                }
            }

            d->foldersToScan = folderList;

            // If we are scanning for newly imported files, we need to have the folders for scanning...

            if (d->foldersToScan.isEmpty())
            {
                qCWarning(DIGIKAM_MAINTENANCE_LOG) << "NewItemsFinder called without any folders. Wrong call.";

                slotDone();

                return;
            }

            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "scan mode: ScheduleCollectionScan :: " << d->foldersToScan;

            connect(ScanController::instance(), SIGNAL(partialScanDone(QString)),
                    this, SLOT(slotPartialScanDone(QString)));

            for (const QString& folder : std::as_const(d->foldersToScan))
            {
                if (d->cancel)
                {
                    break;
                }

                ScanController::instance()->scheduleCollectionScan(folder);
            }

            break;
        }
    }
}

void NewItemsFinder::connectToScanController()
{
    // Common connections to ScanController

    connect(ScanController::instance(), SIGNAL(collectionScanStarted(QString)),
            this, SLOT(slotScanStarted(QString)));

    connect(ScanController::instance(), SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(ScanController::instance(), SIGNAL(startScanningAlbum(QString,QString)),
            this, SLOT(slotStartScanningAlbum(QString,QString)));

    connect(ScanController::instance(), SIGNAL(filesScanned(int)),
            this, SLOT(slotFilesScanned(int)));

    connect(ScanController::instance(), SIGNAL(completeScanDone()),
            this, SLOT(slotDone()));
}

void NewItemsFinder::slotScanStarted(const QString& info)
{
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << info;
    setStatus(info);
}

void NewItemsFinder::slotTotalFilesToScan(int t)
{
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << "increment scan value : " << t;
    incTotalItems(t);
}

void NewItemsFinder::slotStartScanningAlbum(const QString& albumRoot, const QString& album)
{
    QString lbl = i18n("Scanning: %1\n", album);
    lbl.append(i18n("Album Root: %1", albumRoot));
    setLabel(lbl);
}

void NewItemsFinder::slotFilesScanned(int s)
{
/*
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << "file scanned : " << s;
*/
    advance(s);
}

void NewItemsFinder::slotCancel()
{
    d->cancel = true;

    ScanController::instance()->cancelCompleteScan();
    MaintenanceTool::slotCancel();
}

void NewItemsFinder::slotPartialScanDone(const QString& path)
{
    // Check if path scanned is included in planned list.

    if (d->foldersToScan.contains(path) && !d->foldersScanned.contains(path))
    {
        d->foldersScanned.append(path);
        d->foldersScanned.sort();

        QString lbl = i18n("Scanned:\n%1", path);
        setLabel(lbl);

        // Check if all planned scanning is done

        if (d->foldersScanned == d->foldersToScan)
        {
            slotDone();
        }
    }
}

void NewItemsFinder::slotDone()
{
    setThumbnail(QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(48));

    QString lbl;

    if (totalItems() > 1)
    {
        lbl.append(i18n("Items scanned for cataloging: %1", totalItems()));
    }
    else
    {
        lbl.append(i18n("Item scanned for cataloging: %1", totalItems()));
    }

    setLabel(lbl);

    MaintenanceTool::slotDone();
}

} // namespace Digikam

#include "moc_newitemsfinder.cpp"
