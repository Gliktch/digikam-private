/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Face scan settings container.
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facescansettings.h"

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

namespace Digikam
{

FaceScanSettings::FaceScanSettings(const FaceScanSettings& other)
    : alreadyScannedHandling(other.alreadyScannedHandling),
      task                  (other.task),
      useFullCpu            (other.useFullCpu),
      detectModel           (other.detectModel),
      detectSize            (other.detectSize),
      detectAccuracy        (other.detectAccuracy),
      recognizeModel        (other.recognizeModel),
      recognizeAccuracy     (other.recognizeAccuracy),
      wholeAlbums           (other.wholeAlbums),
      albums                (other.albums),
      infos                 (other.infos),
      workerThreadPriority  (other.workerThreadPriority),
      source                (other.source)

{
}

FaceScanSettings& FaceScanSettings::operator=(const FaceScanSettings& other)
{
    alreadyScannedHandling = other.alreadyScannedHandling;
    task                   = other.task;
    useFullCpu             = other.useFullCpu;
    detectModel            = other.detectModel;
    detectSize             = other.detectSize;
    detectAccuracy         = other.detectAccuracy;
    recognizeModel         = other.recognizeModel;
    recognizeAccuracy      = other.recognizeAccuracy;
    wholeAlbums            = other.wholeAlbums;
    albums                 = other.albums;
    infos                  = other.infos;
    workerThreadPriority   = other.workerThreadPriority;
    source                 = other.source;

    return *this;
}

void FaceScanSettings::readFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configName);
    readFromConfig(group);
}

void FaceScanSettings::readFromConfig(const KConfigGroup& group)
{
    alreadyScannedHandling = static_cast<AlreadyScannedHandling>(group.readEntry(configFaceAlreadyScannedHandling, (int)AlreadyScannedHandling::Skip));
    task                   = static_cast<ScanTask>(group.readEntry(configFaceScanTask,                             (int)ScanTask::DetectAndRecognize));
    useFullCpu             = group.readEntry(configUseFullCpu,                                                     true);
    detectModel            = static_cast<FaceDetectionModel>(group.readEntry(configFaceDetectionModel,             (int)FaceDetectionModel::YuNet));
    detectSize             = static_cast<FaceDetectionSize>(group.readEntry(configFaceDetectionSize,               (int)FaceDetectionSize::Large));
    detectAccuracy         = group.readEntry(configFaceDetectionAccuracy,                                          7);
    recognizeModel         = static_cast<FaceRecognitionModel>(group.readEntry(configFaceRecognitionModel,         (int)FaceRecognitionModel::SFace));
    recognizeAccuracy      = group.readEntry(configFaceRecognitionAccuracy,                                        7);

    // NOTE: Album settings are handled by AlbumSelector widget.
}

void FaceScanSettings::writeToConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configName);
    writeToConfig(group);
}

void FaceScanSettings::writeToConfig(KConfigGroup& group)
{
    group.writeEntry(configFaceAlreadyScannedHandling, (int)alreadyScannedHandling);
    group.writeEntry(configFaceScanTask,               (int)task);
    group.writeEntry(configUseFullCpu,                 useFullCpu);
    group.writeEntry(configFaceDetectionModel,         (int)detectModel);
    group.writeEntry(configFaceDetectionSize,          (int)detectSize);
    group.writeEntry(configFaceDetectionAccuracy,      detectAccuracy);
    group.writeEntry(configFaceRecognitionModel,       (int)recognizeModel);
    group.writeEntry(configFaceRecognitionAccuracy,    recognizeAccuracy);

    // NOTE: Album settings are handled by AlbumSelector widget.
}

QDebug operator<<(QDebug dbg, const FaceScanSettings& s)
{
    dbg.nospace()                                                            << Qt::endl;
    dbg.nospace() << "Already Scanned Handling:" << s.alreadyScannedHandling << Qt::endl;
    dbg.nospace() << "Scan Task               :" << s.task                   << Qt::endl;
    dbg.nospace() << "Use Full CPU            :" << s.useFullCpu             << Qt::endl;
    dbg.nospace() << "Detection Model         :" << s.detectModel            << Qt::endl;
    dbg.nospace() << "Detection Size          :" << s.detectSize             << Qt::endl;
    dbg.nospace() << "Detection Accuracy      :" << s.detectAccuracy         << Qt::endl;
    dbg.nospace() << "Recognition Model       :" << s.recognizeModel         << Qt::endl;
    dbg.nospace() << "Recognition Accuracy    :" << s.recognizeAccuracy      << Qt::endl;
    dbg.nospace() << "Whole Albums            :" << s.wholeAlbums            << Qt::endl;
    dbg.nospace() << "Albums                  :" << s.albums.count()         << Qt::endl;
    dbg.nospace() << "Image Infos             :" << s.infos.count()          << Qt::endl;
    dbg.nospace() << "Worker Thread Priority  :" << s.workerThreadPriority   << Qt::endl;
    dbg.nospace() << "Source                  :" << s.source                 << Qt::endl;

    return dbg.space();
}

} // namespace Digikam

#include "moc_facescansettings.cpp"
