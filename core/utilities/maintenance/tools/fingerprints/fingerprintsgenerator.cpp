/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : fingerprints generator
 *
 * SPDX-FileCopyrightText: 2018      by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fingerprintsgenerator.h"

// Qt includes

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QApplication>
#include <QString>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "albummanager.h"
#include "collectionmanager.h"
#include "maintenancethread.h"
#include "dnotificationwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN FingerPrintsGenerator::Private
{
public:

    Private() = default;

public:

    bool                 rebuildAll   = true;

    AlbumList            albumList;

    QList<qlonglong>     allItemIds;

    QFuture<void>        affectedAlbumTask;
    QFutureWatcher<void> affectedAlbumWatcher;

    MaintenanceThread*   thread       = nullptr;
};

FingerPrintsGenerator::FingerPrintsGenerator(const bool rebuildAll, const AlbumList& list, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("FingerPrintsGenerator"), parent),
      d              (new Private)
{
    d->albumList  = list;
    d->rebuildAll = rebuildAll;
    d->thread     = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvanceInfo(ItemInfo,QImage)),
            this, SLOT(slotAdvance(ItemInfo,QImage)));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
    delete d;
}

void FingerPrintsGenerator::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void FingerPrintsGenerator::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void FingerPrintsGenerator::slotStart()
{
    MaintenanceTool::slotStart();

    setThumbnail(QIcon::fromTheme(QLatin1String("fingerprint")).pixmap(48));
    setLabel(i18n("Finger-prints"));

    // Activate progress bar during album calculation.

    setTotalItems(1);

    connect(&d->affectedAlbumWatcher, &QFutureWatcher<void>::finished,
            this, &FingerPrintsGenerator::slotAffectedAlbumsFinished);

    d->affectedAlbumTask =
        QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                          &FingerPrintsGenerator::calculateAffectedAlbums, this

#else

                          this, &FingerPrintsGenerator::calculateAffectedAlbums

#endif

                         );

    d->affectedAlbumWatcher.setFuture(d->affectedAlbumTask);
}

void FingerPrintsGenerator::slotAffectedAlbumsFinished()
{
    if (d->allItemIds.isEmpty())
    {
        slotDone();

        return;
    }

    setTotalItems(d->allItemIds.count());

    d->thread->generateFingerprints(d->allItemIds, d->rebuildAll);
    d->thread->start();
}

void FingerPrintsGenerator::calculateAffectedAlbums()
{
    if (d->albumList.isEmpty())
    {
        d->albumList = AlbumManager::instance()->allPAlbums();
    }

    // Get all item IDs from albums.

    for (AlbumList::ConstIterator it = d->albumList.constBegin() ;
         !canceled() && (it != d->albumList.constEnd()) ; ++it)
    {
        if      ((*it)->type() == Album::PHYSICAL)
        {
            const auto ids = CoreDbAccess().db()->getItemIDsInAlbum((*it)->id());

            for (const qlonglong& id : ids)
            {
                if (!d->allItemIds.contains(id))
                {
                    if ((id == 119119) || (id == 118948) || (id == 118878))
                    {
                        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Add item id" << id;
                    }

                    d->allItemIds << id;
                }
            }
        }
        else if ((*it)->type() == Album::TAG)
        {
            const auto ids = CoreDbAccess().db()->getItemIDsInTag((*it)->id());

            for (const qlonglong& id : ids)
            {
                if (!d->allItemIds.contains(id))
                {
                    d->allItemIds << id;
                }
            }
        }
    }
}

void FingerPrintsGenerator::slotAdvance(const ItemInfo& inf, const QImage& img)
{
    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    QString album = CollectionManager::instance()->albumRootLabel(inf.albumRootId());

    QString lbl = i18n("Rebuild Fingerprint: %1\n", inf.name());
    lbl.append(i18n("Album: %1", album + inf.relativePath()));
    setLabel(lbl);
    setThumbnail(QIcon(QPixmap::fromImage(img)));
    advance(adv);
}

void FingerPrintsGenerator::slotDone()
{
    setThumbnail(QIcon::fromTheme(QLatin1String("fingerprint")).pixmap(48));

    QString lbl;

    if (totalItems() > 1)
    {
        lbl.append(i18n("Items scanned for fingerprint: %1", totalItems()));
    }
    else
    {
        lbl.append(i18n("Item scanned for fingerprint: %1", totalItems()));
    }

    setLabel(lbl);

    // Dispatch scan resume to the icon-view info pop-up.

    Q_EMIT signalScanNotification(lbl, DNotificationWidget::Information);

    // Switch on scanned for finger-prints flag on digiKam config file.

    KSharedConfig::openConfig()->group(QLatin1String("General Settings"))
                                       .writeEntry(QLatin1String("Finger Prints Generator First Run"), true);

    MaintenanceTool::slotDone();
}

} // namespace Digikam

#include "moc_fingerprintsgenerator.cpp"
