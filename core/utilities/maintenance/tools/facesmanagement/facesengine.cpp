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

#include "facesengine_p.h"

namespace Digikam
{

FacesEngine::FacesEngine(const FaceScanSettings& settings, ProgressItem* const parent)
    : MaintenanceTool(faceScanTaskToString(settings.source), parent),
      d              (new Private)
{
    bool incompatibleScanCheck = false;
    bool stopBackgroundProcess = false;
    bool showNotification      = true;

    // check for incompatible running scans

    switch (settings.source)
    {
        case FaceScanSettings::FaceScanSource::FaceScanWidget:
        {
            // FaceScanWidget scans are incompatible with FaceScanWidget, Background, and Maintenance scans

            if (
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::FaceScanWidget)) ||
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::MaintenanceTool))
               )
            {
                incompatibleScanCheck = true;
            }

            stopBackgroundProcess = true;

            break;
        }

        case FaceScanSettings::FaceScanSource::ItemIconView:
        {
            // ItemIconView scans are incompatible with Maintenance scans

            if (
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::MaintenanceTool))
               )
            {
                incompatibleScanCheck = true;
            }

            stopBackgroundProcess = false;

            break;
        }

        case FaceScanSettings::FaceScanSource::MaintenanceTool:
        {
            // Maintenance scans are incompatible with all other scan sources

            if (
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::FaceScanWidget)) ||
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::ItemIconView)) ||
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::MaintenanceTool)) ||
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::BQM))
               )
            {
                incompatibleScanCheck = true;
            }

            stopBackgroundProcess = true;

            break;
        }

        case FaceScanSettings::FaceScanSource::BackgroundRecognition:
        {
            // Background scans are incompatible with FaceScanWidget, BackgroundRecognition, and Maintenance scans
            // instead of showing an error message, we just return without doing anything

            if (
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::FaceScanWidget)) ||
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::MaintenanceTool)) ||
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::BackgroundRecognition))
               )
            {
                incompatibleScanCheck = true;
                showNotification      = false;
            }

            break;
        }

        case FaceScanSettings::FaceScanSource::BQM:
        {
            // Background scans are incompatible with Maintenance scans

            if (
                ProgressManager::instance()->findItembyId(faceScanTaskToString(FaceScanSettings::FaceScanSource::MaintenanceTool))
               )
            {
                incompatibleScanCheck = true;
            }

            stopBackgroundProcess = false;

            break;
        }
    }

    // show error message if incompatible scan is running

    if (incompatibleScanCheck)
    {
        // show error message if incompatible scan is running

        if (showNotification)
        {
            QString message = i18n("A face scan is already running. "
                                   "Only one face task can be running at a time. "
                                   "Please wait until it is finished.");

            Q_EMIT DigikamApp::instance()->signalNotificationError(message, DNotificationWidget::Information);
/*
            Q_EMIT signalScanNotification(message, DNotificationWidget::Error);
*/
        }

        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "FacesEngine::FacesEngine: scan already running";
        throw new QException();
    }

    if (stopBackgroundProcess)
    {
        // stop any background process

        FaceRecognitionBackgroundController::instance()->stop();
        FaceRecognitionBackgroundController::instance()->waitForDone();
    }

    // suppress notifications if this is a background process

    if (settings.source == FaceScanSettings::FaceScanSource::BackgroundRecognition)
    {
        setNotificationEnabled(false);
    }

    // select scan type

    switch (settings.task)
    {
        case FaceScanSettings::DetectAndRecognize:
        {
            d->newPipeline = new FacePipelineDetectRecognize(settings);
            break;
        }

        case FaceScanSettings::RecognizeMarkedFaces:
        {
            d->newPipeline = new FacePipelineRecognize(settings);
            break;
        }

        case FaceScanSettings::RetrainAll:
        {
            d->newPipeline = new FacePipelineRetrain(settings);
            break;
        }

        case FaceScanSettings::Reset:
        {
            d->newPipeline = new FacePipelineReset(settings);
            break;
        }
    }

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

    if      (
             settings.wholeAlbums     &&
             settings.infos.isEmpty() &&
             (settings.task == FaceScanSettings::RecognizeMarkedFaces)
            )
    {
        d->idsTodoList   = CoreDbAccess().db()->getImagesWithImageTagProperty(FaceTags::unknownPersonTagId(),
                                                                              ImageTagPropertyName::autodetectedFace());

        d->source        = FacesEngine::Ids;
    }
    else if (settings.task == FaceScanSettings::RetrainAll)
    {
        d->idsTodoList   = CoreDbAccess().db()->getImagesWithProperty(ImageTagPropertyName::tagRegion());

        d->source        = FacesEngine::Ids;
    }
    else if (settings.albums.isEmpty() && settings.infos.isEmpty())
    {
        d->albumTodoList = AlbumManager::instance()->allPAlbums();
        d->source        = FacesEngine::Albums;
    }
    else if (!settings.albums.isEmpty())
    {
        d->albumTodoList = settings.albums;
        d->source        = FacesEngine::Albums;
    }
    else
    {
        d->infoTodoList  = settings.infos;
        d->source        = FacesEngine::Infos;
    }
}

FacesEngine::~FacesEngine()
{
    delete d->newPipeline;
    delete d;
}

void FacesEngine::slotStart()
{
    MaintenanceTool::slotStart();

    d->totalFacesFound = 0;

    setThumbnail(QIcon::fromTheme(QLatin1String("edit-image-face-detect")).pixmap(48));

    // Set label depending on settings.

    if      (d->albumTodoList.size() > 0)
    {
        if (d->albumTodoList.size() == 1)
        {
            setLabel(i18n("Scan for faces in album: %1", d->albumTodoList.first()->title()));
        }
        else
        {
            setLabel(i18n("Scan for faces in %1 albums", d->albumTodoList.size()));
        }
    }
    else if (d->infoTodoList.size() > 0)
    {
        if (d->infoTodoList.size() == 1)
        {
            setLabel(i18n("Scan for faces in image: %1", d->infoTodoList.first().name()));
        }
        else
        {
            setLabel(i18n("Scan for faces in %1 images", d->infoTodoList.size()));
        }
    }
    else
    {
        setLabel(i18n("Updating faces database"));
    }

    if      (d->source == FacesEngine::Infos)
    {
        int total = d->infoTodoList.count();
        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Total is" << total;

        setTotalItems(total);

        if (d->infoTodoList.isEmpty())
        {
            slotDone();

            return;
        }

        if (!d->newPipeline->start())
        {
            Q_EMIT signalScanNotification(QString(i18n("Error starting face detection.")),
                                          DNotificationWidget::Error);

            slotDone();

            return;
        }

        return;
    }
    else if (d->source == FacesEngine::Ids)
    {
        ItemInfoList itemInfos(d->idsTodoList);

        int total = itemInfos.count();
        qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Total is" << total;

        setTotalItems(total);

        if (itemInfos.isEmpty())
        {
            slotDone();

            return;
        }

        if (!d->newPipeline->start())
        {
            Q_EMIT signalScanNotification(QString(i18n("Error starting face detection.")),
                                          DNotificationWidget::Error);
        }

        return;
    }

    setUsesBusyIndicator(true);

    // Get total count, cached by AlbumManager.

    QHash<int, int> palbumCounts;
    QHash<int, int> talbumCounts;
    bool hasPAlbums = false;
    bool hasTAlbums = false;

    for (const Album* const album : std::as_const(d->albumTodoList))
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

    for (Album* const album : std::as_const(d->albumTodoList))
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
    qCDebug(DIGIKAM_MAINTENANCE_LOG) << "Total is" << total;

    setUsesBusyIndicator(false);
    setTotalItems(total);

    if (!d->newPipeline->start())
    {
        Q_EMIT signalScanNotification(QString(i18n("Error starting face detection.")),
                                      DNotificationWidget::Error);
    }
}

void FacesEngine::slotUpdateItemCount(const qlonglong itemCount)
{
    setTotalItems(itemCount);
}

void FacesEngine::slotDone()
{
/*
    if (d->benchmark)
    {
        new BenchmarkMessageDisplay(d->pipeline.benchmarkResult());
    }
*/
    QString lbl;

    if (totalItems() > 1)
    {
        lbl.append(i18n("Items scanned for faces: %1\n", totalItems()));
    }
    else
    {
        lbl.append(i18n("Item scanned for faces: %1\n", totalItems()));
    }

    if (d->totalFacesFound > 1)
    {
        lbl.append(i18n("Faces found: %1", d->totalFacesFound));
    }
    else
    {
        lbl.append(i18n("Face found: %1", d->totalFacesFound));
    }

    setLabel(lbl);

    // Dispatch scan resume to the icon-view info pop-up.

    Q_EMIT signalScanNotification(lbl, DNotificationWidget::Information);

    // Switch on scanned for faces flag on digiKam config file.

    KSharedConfig::openConfig()->group(QLatin1String("General Settings"))
                                       .writeEntry("Face Scanner First Run", true);

    MaintenanceTool::slotDone();
}

void FacesEngine::slotCancel()
{
    d->newPipeline->cancel();
    MaintenanceTool::slotCancel();
}

void FacesEngine::slotImagesSkipped(const MLPipelinePackageNotify::Ptr& package)
{
    Q_UNUSED(package);

    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    advance(adv);
}

void FacesEngine::slotShowOneDetected(const MLPipelinePackageNotify::Ptr& package)
{
    uint adv = 0;

    if ((adv = checkProgressNeeded()) == 0)
    {
        return;
    }

    setThumbnail(package->thumbnail);

    QString lbl = i18n("Face Scanning: %1\n", package->name);
    lbl.append(i18n("Album: %1\n", package->path));

    if (package->processed > 0)
    {
        if (package->processed == 1)
        {
            lbl.append(i18n("Found %1 face.", package->processed));
        }
        else
        {
            lbl.append(i18n("Found %1 faces.", package->processed));
        }

        if (!package->displayData.isEmpty())
        {
            lbl.append(QStringLiteral("  "));
            lbl.append(i18n("Recognized: %1", package->displayData));
        }
    }
/*
    else
    {
        lbl.append(i18n("No faces found."));
    }
*/
    d->totalFacesFound += package->processed;

    setLabel(lbl);
    advance(adv);
}

QString FacesEngine::faceScanTaskToString(FaceScanSettings::FaceScanSource source)
{
    QString faceStr = QLatin1String("FacesEngine");

    switch (source)
    {
        case FaceScanSettings::FaceScanSource::FaceScanWidget:
        {
            faceStr += QLatin1String(" (FaceScanWidget)");
            break;
        }

        case FaceScanSettings::FaceScanSource::ItemIconView:
        {
            faceStr += QLatin1String(" (ItemIconView)");
            break;
        }

        case FaceScanSettings::FaceScanSource::MaintenanceTool:
        {
            faceStr += QLatin1String(" (MaintenanceTool)");
            break;
        }

        case FaceScanSettings::FaceScanSource::BackgroundRecognition:
        {
            faceStr += QLatin1String(" (BackgroundRecognition)");
            break;
        }

        case FaceScanSettings::FaceScanSource::BQM:
        {
            faceStr += QLatin1String(" (BQM)");
            break;
        }
    }

    return faceStr;
}

FaceScanSettings::FaceScanSource FacesEngine::faceScanTaskToEnum(const QString& taskName)
{
    if      (QLatin1String("FacesEngine (FaceScanWidget)") == taskName)
    {
        return FaceScanSettings::FaceScanSource::FaceScanWidget;
    }
    else if (QLatin1String("FacesEngine (ItemIconView)") == taskName)
    {
        return FaceScanSettings::FaceScanSource::ItemIconView;
    }
    else if (QLatin1String("FacesEngine (MaintenanceTool)") == taskName)
    {
        return FaceScanSettings::FaceScanSource::MaintenanceTool;
    }
    else if (QLatin1String("FacesEngine (BackgroundRecognition)") == taskName)
    {
        return FaceScanSettings::FaceScanSource::BackgroundRecognition;
    }
    else if (QLatin1String("FacesEngine (BQM)") == taskName)
    {
        return FaceScanSettings::FaceScanSource::BQM;
    }

    return FaceScanSettings::FaceScanWidget;
}

} // namespace Digikam

#include "moc_facesengine.cpp"
