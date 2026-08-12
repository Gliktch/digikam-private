/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Database cleaner.
 *
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbcleaner.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QThread>
#include <QVBoxLayout>
#include <QTimer>
#include <QListWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikamapp.h"
#include "maintenancethread.h"
#include "dbshrinkdialog.h"

namespace Digikam
{

class Q_DECL_HIDDEN DbCleaner::Private
{
public:

    Private() = default;

public:

    MaintenanceThread* thread                   = nullptr;
    bool               cleanThumbsDb            = false;
    bool               cleanFacesDb             = false;
    bool               cleanSimilarityDb        = false;
    bool               shrinkDatabases          = false;

    QList<qlonglong>   imagesToRemove;
    QList<int>         staleThumbnails;
    QList<Identity>    staleIdentities;
    QList<qlonglong>   staleImageSimilarities;

    int                databasesToAnalyseCount  = 1;
    int                databasesToShrinkCount   = 0;

    DbShrinkDialog*    shrinkDlg                = nullptr;
};

DbCleaner::DbCleaner(bool cleanThumbsDb,
                     bool cleanFacesDb,
                     bool cleanSimilarityDb,
                     bool shrinkDatabases,
                     ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("DbCleaner"), parent),
      d              (new Private)
{
    // register the identity list as meta type to be able to use it in signal/slot connection

    qRegisterMetaType<QList<Identity> >("QList<Identity>");

    d->cleanThumbsDb     = cleanThumbsDb;

    if (cleanThumbsDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->cleanFacesDb      = cleanFacesDb;

    if (cleanFacesDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->cleanSimilarityDb = cleanSimilarityDb;

    if (cleanSimilarityDb)
    {
        d->databasesToAnalyseCount = d->databasesToAnalyseCount + 1;
    }

    d->shrinkDatabases   = shrinkDatabases;

    if (shrinkDatabases)
    {
        d->databasesToShrinkCount = 4;
        d->shrinkDlg              = new DbShrinkDialog(DigikamApp::instance());
    }

    d->thread            = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalAdvance()),
            this, SLOT(slotAdvance()));
}

DbCleaner::~DbCleaner()
{
    delete d->shrinkDlg;
    delete d;
}

void DbCleaner::slotStart()
{
    MaintenanceTool::slotStart();

    setLabel(i18n("Clean up the databases : ") + i18n("analysing databases"));
    setThumbnail(QIcon::fromTheme(QLatin1String("tools-wizard")).pixmap(48));

    // Set one item to make sure that the progress bar is shown.

    setTotalItems(d->databasesToAnalyseCount);
/*
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Completed items at start: "
                                 << completedItems() << "/"
                                 << totalItems();
*/
    // Set the wiring from the data signal to the data slot.

    connect(d->thread, SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)),
            this, SLOT(slotFetchedData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)));

    connect(d->thread, SIGNAL(signalAddItemsToProcess(int)),
            this, SLOT(slotAddItemsToProcess(int)));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotCleanItems()));

    // Compute the database junk. This will lead to the call of the slot slotFetchedData.

    d->thread->computeDatabaseJunk(d->cleanThumbsDb, d->cleanFacesDb, d->cleanSimilarityDb);
    d->thread->start();
}

void DbCleaner::slotAddItemsToProcess(int count)
{
    incTotalItems(count);
}

void DbCleaner::slotFetchedData(const QList<qlonglong>& staleImageIds,
                                const QList<int>& staleThumbIds,
                                const QList<Identity>& staleIdentities,
                                const QList<qlonglong>& staleImageSimilarities)
{
    // We have data now. Store it and trigger the core db cleaning

    d->imagesToRemove         = staleImageIds;
    d->staleThumbnails        = staleThumbIds;
    d->staleIdentities        = staleIdentities;
    d->staleImageSimilarities = staleImageSimilarities;

    // If we have nothing to do, finish.
    // Signal done if no elements cleanup is necessary

    if (
        d->imagesToRemove.isEmpty()       &&
        d->staleThumbnails.isEmpty()      &&
        d->staleIdentities.isEmpty()      &&
        d->staleImageSimilarities.isEmpty()
       )
    {
        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Nothing to do. Databases are clean.";

        if (d->shrinkDatabases)
        {
            disconnect(d->thread, SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)),
                       this, SLOT(slotFetchedData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)));

            disconnect(d->thread, SIGNAL(signalCompleted()),
                       this, SLOT(slotCleanItems()));

            slotShrinkDatabases();

            return;
        }
        else
        {
            MaintenanceTool::slotDone();

            return;
        }
    }

    reset();
    setTotalItems(
                  d->imagesToRemove.size()       +
                  d->staleThumbnails.size()      +
                  d->staleIdentities.size()      +
                  d->staleImageSimilarities.size()
                 );
/*
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Completed items after analysis: " << completedItems() << "/" << totalItems();
*/
}

void DbCleaner::slotCleanItems()
{
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Cleaning core db.";

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanItems()));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotCleanedItems()));

    if (d->imagesToRemove.size() > 0)
    {
        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Found" << d->imagesToRemove.size() << "obsolete image entries.";

        setLabel(i18n("Clean up the databases : ") + i18n("cleaning core db"));

        // GO!

        d->thread->cleanCoreDb(d->imagesToRemove);
        d->thread->start();
    }
    else
    {
        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Core DB is clean.";
        slotCleanedItems();
    }
}

void DbCleaner::slotCleanedItems()
{
    // We cleaned the items. Now clean the thumbs db

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedItems()));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotCleanedThumbnails()));

    if (d->cleanThumbsDb)
    {
        if (!d->staleThumbnails.isEmpty())
        {
            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Found" << d->staleThumbnails.size() << "stale thumbnails.";
            setLabel(i18n("Clean up the databases : ") + i18n("cleaning thumbnails db"));

            // GO!

            d->thread->cleanThumbsDb(d->staleThumbnails);
            d->thread->start();
        }
        else
        {
            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Thumbnail DB is clean.";
            slotCleanedThumbnails();
        }
    }
    else
    {
        slotCleanedThumbnails();
    }
}

void DbCleaner::slotCleanedThumbnails()
{
    // We cleaned the thumbnails. Now clean the recognition db

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedThumbnails()));

    if (d->cleanFacesDb)
    {
        if (d->staleIdentities.count() > 0)
        {
            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Found" << d->staleIdentities.size() << "stale face identities.";
            setLabel(i18n("Clean up the databases : ") + i18n("cleaning recognition db"));

            // GO! and don't forget the signal!

            connect(d->thread, SIGNAL(signalCompleted()),
                    this, SLOT(slotCleanedFaces()));

            // We cleaned the thumbs db. Now clean the faces db.

            d->thread->cleanFacesDb(d->staleIdentities);
            d->thread->start();
        }
        else
        {
            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Faces DB is clean.";
            slotCleanedFaces();
        }
    }
    else
    {
        slotCleanedFaces();
    }
}

void DbCleaner::slotCleanedFaces()
{
    // We cleaned the recognition db. Now clean the similarity db

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedFaces()));

    if (d->cleanSimilarityDb)
    {
        // TODO: implement similarity db cleanup

        if (!d->staleImageSimilarities.isEmpty())
        {
            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Found" << d->staleImageSimilarities.size()
                                         << "image ids that are referenced in similarity db but not used.";
            setLabel(i18n("Clean up the databases : ") + i18n("cleaning similarity db"));

            // GO! and don't forget the signal!

            connect(d->thread, SIGNAL(signalCompleted()),
                    this, SLOT(slotCleanedSimilarity()));

            // We cleaned the thumbs db. Now clean the faces db.

            d->thread->cleanSimilarityDb(d->staleImageSimilarities);
            d->thread->start();
        }
        else
        {
            qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Similarity DB is clean.";
            slotCleanedSimilarity();
        }
    }
    else
    {
        slotCleanedSimilarity();
    }
}

void DbCleaner::slotCleanedSimilarity()
{
    // We cleaned the similarity db. We are done.

    disconnect(d->thread, SIGNAL(signalCompleted()),
               this, SLOT(slotCleanedSimilarity()));

    if (d->shrinkDatabases)
    {
        slotShrinkDatabases();

        return;
    }

    slotDone();
}

void DbCleaner::slotShrinkDatabases()
{
    reset();
    setTotalItems(d->databasesToShrinkCount);
    setLabel(i18n("Clean up the databases : ") + i18n("shrinking databases"));

    connect(d->thread, SIGNAL(signalFinished(bool,bool)),
            this, SLOT(slotShrinkNextDBInfo(bool,bool)));

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    d->thread->shrinkDatabases();

    d->shrinkDlg->open();

    d->thread->start();
}

void DbCleaner::slotAdvance()
{
    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    advance(adv);
}

void DbCleaner::slotShrinkNextDBInfo(bool done, bool passed)
{
    --d->databasesToShrinkCount;

    QIcon statusIcon = QIcon::fromTheme(QLatin1String("dialog-cancel"));

    if (done)
    {
        if (passed)
        {
            statusIcon = QIcon::fromTheme(QLatin1String("dialog-ok-apply"));
        }
        else
        {
            statusIcon = QIcon::fromTheme(QLatin1String("script-error"));
        }
    }

    switch (d->databasesToShrinkCount)
    {
        case 3:
        {
            d->shrinkDlg->setIcon(0, statusIcon);
            d->shrinkDlg->setActive(1);
            break;
        }

        case 2:
        {
            d->shrinkDlg->setIcon(1, statusIcon);
            d->shrinkDlg->setActive(2);
            break;
        }

        case 1:
        {
            d->shrinkDlg->setIcon(2, statusIcon);
            d->shrinkDlg->setActive(3);
            break;
        }

        case 0:
        {
            d->shrinkDlg->setIcon(3, statusIcon);
            d->shrinkDlg->setActive(-1);
            break;
        }
    }

    advance(1);
}

void DbCleaner::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void DbCleaner::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void DbCleaner::slotDone()
{
    if (d->shrinkDlg)
    {
        d->shrinkDlg->close();
    }

    MaintenanceTool::slotDone();
}

} // namespace Digikam

#include "moc_dbcleaner.cpp"
