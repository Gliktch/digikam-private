/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-01-06
 * Description : autotags scanning settings container
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autotagsscansettings.h"

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

namespace Digikam
{

AutotagsScanSettings::AutotagsScanSettings(const AutotagsScanSettings& other)
    : scanMode                (other.scanMode),
      tagMode                 (other.tagMode),
      useFullCpu              (other.useFullCpu),
      objectDetectModel       (other.objectDetectModel),
      uiConfidenceThreshold   (other.uiConfidenceThreshold),
      languages               (other.languages),
      wholeAlbums             (other.wholeAlbums),
      albums                  (other.albums),
      bqmMode                 (other.bqmMode)
{
}

AutotagsScanSettings& AutotagsScanSettings::operator=(const AutotagsScanSettings& other)
{
    scanMode                 = other.scanMode;
    tagMode                  = other.tagMode;
    useFullCpu               = other.useFullCpu;
    objectDetectModel        = other.objectDetectModel;
    uiConfidenceThreshold    = other.uiConfidenceThreshold;
    languages                = other.languages;
    wholeAlbums              = other.wholeAlbums;
    albums                   = other.albums;
    bqmMode                  = other.bqmMode;

    return *this;
}

void AutotagsScanSettings::readFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configName);
    readFromConfig(group);
}

void AutotagsScanSettings::readFromConfig(const KConfigGroup& group)
{
    scanMode              = (ScanMode)group.readEntry(configScanMode,   (int)ScanMode::AllItems);
    tagMode               = (TagMode)group.readEntry(configTagMode,     (int)TagMode::Replace);
    useFullCpu            = group.readEntry(configUseFullCpu,           false);
    objectDetectModel     = group.readEntry(configObjectDetectModel,    QString::fromLatin1("yolov11-nano"));
    uiConfidenceThreshold = group.readEntry(configObjectDetectAccuracy, 7);
    languages             = group.readEntry(configLanguages,            QStringList());

    // NOTE: Album settings are handled by AlbumSelector widget.
}

void AutotagsScanSettings::writeToConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configName);
    writeToConfig(group);
}

void AutotagsScanSettings::writeToConfig(KConfigGroup& group)
{
    group.writeEntry(configScanMode,             (int)scanMode);
    group.writeEntry(configTagMode,              (int)tagMode);
    group.writeEntry(configUseFullCpu,           useFullCpu);
    group.writeEntry(configObjectDetectModel,    objectDetectModel);
    group.writeEntry(configObjectDetectAccuracy, (int)uiConfidenceThreshold);
    group.writeEntry(configLanguages,            languages);

    // NOTE: Album settings are handled by AlbumSelector widget.
}

QDebug operator<<(QDebug dbg, const AutotagsScanSettings& s)
{
    dbg.nospace()                                                                << Qt::endl;
    dbg.nospace() << "Scan Mode                 :" << s.scanMode                 << Qt::endl;
    dbg.nospace() << "Tag Mode                  :" << s.tagMode                  << Qt::endl;
    dbg.nospace() << "Use Full CPU              :" << s.useFullCpu               << Qt::endl;
    dbg.nospace() << "Object Detect Model       :" << s.objectDetectModel        << Qt::endl;
    dbg.nospace() << "Confidence Threshold      :" << s.uiConfidenceThreshold    << Qt::endl;
    dbg.nospace() << "Languages                 :" << s.languages                << Qt::endl;
    dbg.nospace() << "Whole Albums              :" << s.wholeAlbums              << Qt::endl;
    dbg.nospace() << "Albums                    :" << s.albums.count()           << Qt::endl;
    dbg.nospace() << "Bqm Mode                  :" << s.bqmMode                  << Qt::endl;

    return dbg.space();
}

} // namespace Digikam

#include "moc_autotagsscansettings.cpp"
