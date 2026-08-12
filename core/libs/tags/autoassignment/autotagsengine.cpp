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

#include "autotagsengine.h"

// Qt includes

#include <QIcon>
#include <QHash>
#include <QPixmap>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "album.h"
#include "albummanager.h"
#include "dnotificationwidget.h"
#include "autotagsscansettings.h"
#include "mlpipelinepackagenotify.h"
#include "autotagspipelineobject.h"

namespace Digikam
{

class Q_DECL_HIDDEN AutotagsEngine::Private
{
public:

    Private() = default;

public:

    AutotagsScanSettings            settings;

    AutotagsPipelineBase*           newPipeline = nullptr;
};

AutotagsEngine::AutotagsEngine(const AutotagsScanSettings& _settings, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("AutotagsEngine"), parent),
      d              (new Private)
{
    d->settings    = _settings;
    d->newPipeline = new AutotagsPipelineObject(_settings);

    connect(d->newPipeline, SIGNAL(finished()),
            this, SLOT(slotDone()));

    connect(d->newPipeline, SIGNAL(processed(MLPipelinePackageNotify::Ptr)),
            this, SLOT(slotShowOneDetected(MLPipelinePackageNotify::Ptr)));

    connect(d->newPipeline, SIGNAL(skipped(MLPipelinePackageNotify::Ptr)),
            this, SLOT(slotImagesSkipped(MLPipelinePackageNotify::Ptr)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    connect(d->newPipeline, SIGNAL(signalUpdateItemCount(qlonglong)),
            this, SLOT(slotUpdateItemCount(qlonglong)));
}

AutotagsEngine::~AutotagsEngine()
{
    delete d->newPipeline;
    delete d;
}

void AutotagsEngine::slotStart()
{
    MaintenanceTool::slotStart();

    setLabel(i18n("Autotags Assignment"));
    setThumbnail(QIcon::fromTheme(QLatin1String("tag")).pixmap(48));

    // Set label depending on settings.

    if (d->settings.albums.size() > 0)
    {
        if (d->settings.albums.size() == 1)
        {
            setLabel(i18n("Scan for objects in album: %1", d->settings.albums.first()->title()));
        }
        else
        {
            setLabel(i18n("Scan for objects in %1 albums", d->settings.albums.size()));
        }
    }

    setUsesBusyIndicator(true);

    // Get total count, cached by AlbumManager.

    QHash<int, int> palbumCounts;
    QHash<int, int> talbumCounts;
    bool hasPAlbums = false;
    bool hasTAlbums = false;

    for (const Album* const album : std::as_const(d->settings.albums))
    {
        if (album->type() == Album::PHYSICAL)
        {
            hasPAlbums = true;
        }
        else
        {
            hasTAlbums = true;
        }
    }

    palbumCounts = AlbumManager::instance()->getPAlbumsCount();
    talbumCounts = AlbumManager::instance()->getTAlbumsCount();

    if (palbumCounts.isEmpty() && hasPAlbums)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        palbumCounts = CoreDbAccess().db()->getNumberOfImagesInAlbums();
        QApplication::restoreOverrideCursor();
    }

    if (talbumCounts.isEmpty() && hasTAlbums)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        talbumCounts = CoreDbAccess().db()->getNumberOfImagesInTags();
        QApplication::restoreOverrideCursor();
    }

    // First, we use the progressValueMap map to store absolute counts.

    QHash<Album*, int> progressValueMap;

    for (Album* const album : std::as_const(d->settings.albums))
    {
        if (album->type() == Album::PHYSICAL)
        {
            progressValueMap[album] = palbumCounts.value(album->id());
        }
        else
        {
            // This is possibly broken of course because we do not know if images have multiple tags,
            // but there's no better solution without expensive operation.

            progressValueMap[album] = talbumCounts.value(album->id());
        }
    }

    // Second, calculate (approximate) overall sum.

    int total = 0;

    for (int count : std::as_const(progressValueMap))
    {
        // cppcheck-suppress useStlAlgorithm
        total += count;
    }

    total = qMax(1, total);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Total is" << total;

    setUsesBusyIndicator(false);
    setTotalItems(total);

    if (!d->newPipeline->start())
    {
        Q_EMIT signalScanNotification(QString(i18n("Error starting object detection.")), DNotificationWidget::Error);
    }
}

void AutotagsEngine::slotUpdateItemCount(const qlonglong itemCount)
{
    setTotalItems(itemCount);
}

void AutotagsEngine::slotDone()
{
    QString lbl;

    if (totalItems() > 1)
    {
        lbl.append(i18n("Items scanned for objects: %1\n", totalItems()));
    }
    else
    {
        lbl.append(i18n("Item scanned for objects: %1\n", totalItems()));
    }

    setLabel(lbl);

    // Dispatch scan resume to the icon-view info pop-up.

    Q_EMIT signalScanNotification(lbl, DNotificationWidget::Information);

    MaintenanceTool::slotDone();
}

void AutotagsEngine::slotCancel()
{
    d->newPipeline->cancel();
    MaintenanceTool::slotCancel();
}

void AutotagsEngine::slotImagesSkipped(const MLPipelinePackageNotify::Ptr& package)
{
    Q_UNUSED(package);
    advance(1);
}

void AutotagsEngine::slotShowOneDetected(const MLPipelinePackageNotify::Ptr& package)
{
    setThumbnail(package->thumbnail);

    QString lbl = i18n("Object Scanning: %1\n", package->name);
    lbl.append(i18n("Album: %1\n", package->path));

    if (package->processed > 0)
    {
        if (package->processed == 1)
        {
            lbl.append(i18n("Found %1 object. Tag: %2", package->processed, package->displayData));
        }
        else
        {
            lbl.append(i18n("Found %1 objects. Tags: %2", package->processed, package->displayData));
        }
    }

    setLabel(lbl);
    advance(1);
}

} // namespace Digikam

#include "moc_autotagsengine.cpp"
