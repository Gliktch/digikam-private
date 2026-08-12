/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

DMetadata::DMetadata()
    : MetaEngine()
{
    registerMetadataSettings();
}

DMetadata::DMetadata(const QString& filePath)
    : MetaEngine()
{
    registerMetadataSettings();
    load(filePath);
}

DMetadata::DMetadata(const MetaEngineData& data)
    : MetaEngine(data)
{
    registerMetadataSettings();
}

void DMetadata::registerMetadataSettings()
{
    setSettings(MetaEngineSettings::instance()->settings());
}

void DMetadata::setSettings(const MetaEngineSettingsContainer& settings)
{
    setUseXMPSidecar4Reading(settings.useXMPSidecar4Reading);
    setUseCompatibleFileName(settings.useCompatibleFileName);
    setReadWithExifTool(settings.readWithExifTool);
    setWriteWithExifTool(settings.writeWithExifTool);
    setWriteRawFiles(settings.writeRawFiles);
    setWriteDngFiles(settings.writeDngFiles);
    setMetadataWritingMode(settings.metadataWritingMode);
    setUpdateFileTimeStamp(settings.updateFileTimeStamp);
}

} // namespace Digikam
