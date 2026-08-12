/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : MetaEngine Settings Container.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaenginesettingscontainer.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "metaenginesettings.h"
#include "digikam_globals.h"

namespace Digikam
{

void MetaEngineSettingsContainer::readFromConfig(const KConfigGroup& group)
{
    exifRotate            = group.readEntry("EXIF Rotate",                              true);
    exifSetOrientation    = group.readEntry("EXIF Set Orientation",                     true);

    saveTags              = group.readEntry("Save Tags",                                false);
    saveTemplate          = group.readEntry("Save Template",                            false);
    saveFaceTags          = group.readEntry("Save FaceTags",                            false);
    savePosition          = group.readEntry("Save Position",                            false);

    saveComments          = group.readEntry("Save EXIF Comments",                       false);
    saveDateTime          = group.readEntry("Save Date Time",                           false);
    savePickLabel         = group.readEntry("Save Pick Label",                          false);
    saveColorLabel        = group.readEntry("Save Color Label",                         false);
    saveRating            = group.readEntry("Save Rating",                              false);

    readWithExifTool      = group.readEntry("Read Metadata From Files With ExifTool",   false);
    writeWithExifTool     = group.readEntry("Write Metadata To Files With ExifTool",    false);
    writeRawFiles         = group.readEntry("Write Metadata To RAW Files",              false);
    writeDngFiles         = group.readEntry("Write Metadata To DNG Files",              false);

    useXMPSidecar4Reading = group.readEntry("Use XMP Sidecar For Reading",              false);
    useCompatibleFileName = group.readEntry("Use Compatible File Name",                 false);
    metadataWritingMode   = (MetaEngine::MetadataWritingMode)
                            group.readEntry("Metadata Writing Mode",                    (int)MetaEngine::WRITE_TO_FILE_ONLY);
    updateFileTimeStamp   = group.readEntry("Update File Timestamp",                    true);
    rescanImageIfModified = group.readEntry("Rescan File If Modified",                  false);
    useLazySync           = group.readEntry("Use Lazy Synchronization",                 false);
    useFastScan           = group.readEntry("Use Fast Scan At Startup",                 false);

    rotationBehavior      = NoRotation;

    sidecarExtensions     = group.readEntry("Custom Sidecar Extensions",   QStringList());

    exifToolPath          = group.readEntry("ExifTool Path",               defaultExifToolSearchPaths().constFirst());

    if (group.readEntry("Rotate By Internal Flag", true))
    {
        rotationBehavior |= RotateByInternalFlag;
    }

    if (group.readEntry("Rotate By Metadata Flag", true))
    {
        rotationBehavior |= RotateByMetadataFlag;
    }

    if (group.readEntry("Rotate Contents Lossless", true))
    {
        rotationBehavior |= RotateByLosslessRotation;
    }

    if (group.readEntry("Rotate Contents Lossy", false))
    {
        rotationBehavior |= RotateByLossyRotation;
    }

    albumDateFrom         = (AlbumDateSource)
                            group.readEntry("Album Date Source",           (int)OldestItemDate);
}

void MetaEngineSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("EXIF Rotate",                             exifRotate);
    group.writeEntry("EXIF Set Orientation",                    exifSetOrientation);

    group.writeEntry("Save Tags",                               saveTags);
    group.writeEntry("Save Template",                           saveTemplate);
    group.writeEntry("Save FaceTags",                           saveFaceTags);
    group.writeEntry("Save Position",                           savePosition);

    group.writeEntry("Save EXIF Comments",                      saveComments);
    group.writeEntry("Save Date Time",                          saveDateTime);
    group.writeEntry("Save Pick Label",                         savePickLabel);
    group.writeEntry("Save Color Label",                        saveColorLabel);
    group.writeEntry("Save Rating",                             saveRating);

    group.writeEntry("Read Metadata From Files With ExifTool",  readWithExifTool);
    group.writeEntry("Write Metadata To Files With ExifTool",   writeWithExifTool);
    group.writeEntry("Write Metadata To RAW Files",             writeRawFiles);
    group.writeEntry("Write Metadata To DNG Files",             writeDngFiles);

    group.writeEntry("Use XMP Sidecar For Reading",             useXMPSidecar4Reading);
    group.writeEntry("Use Compatible File Name",                useCompatibleFileName);
    group.writeEntry("Metadata Writing Mode",                   (int)metadataWritingMode);
    group.writeEntry("Update File Timestamp",                   updateFileTimeStamp);
    group.writeEntry("Rescan File If Modified",                 rescanImageIfModified);

    group.writeEntry("Rotate By Internal Flag",                 bool(rotationBehavior & RotateByInternalFlag));
    group.writeEntry("Rotate By Metadata Flag",                 bool(rotationBehavior & RotateByMetadataFlag));
    group.writeEntry("Rotate Contents Lossless",                bool(rotationBehavior & RotateByLosslessRotation));
    group.writeEntry("Rotate Contents Lossy",                   bool(rotationBehavior & RotateByLossyRotation));
    group.writeEntry("Album Date Source",                       (int)albumDateFrom);
    group.writeEntry("Use Lazy Synchronization",                useLazySync);
    group.writeEntry("Use Fast Scan At Startup",                useFastScan);

    group.writeEntry("Custom Sidecar Extensions",               sidecarExtensions);

    group.writeEntry("ExifTool Path",                           exifToolPath);
}

bool MetaEngineSettingsContainer::isMetadataWriteEnabled() const
{
    return (
            saveTags       ||
            saveTemplate   ||
            saveFaceTags   ||
            savePosition   ||
            saveComments   ||
            saveDateTime   ||
            savePickLabel  ||
            saveColorLabel ||
            saveRating
           );
}

QStringList MetaEngineSettingsContainer::defaultExifToolSearchPaths() const
{
    QStringList defPaths;

#ifdef Q_OS_MACOS

    // Install path for the official ExifTool DMG package

    defPaths << QLatin1String("/usr/local/bin");

    // digiKam Bundle PKG install path

    defPaths << macOSBundlePrefix() + QLatin1String("bin");

    // Std Macports install path

    defPaths << QLatin1String("/opt/local/bin");

#endif

#ifdef Q_OS_WIN

    defPaths << QLatin1String("C:/Program Files/digiKam");

#endif

#ifdef Q_OS_UNIX

    defPaths << QLatin1String("/usr/bin");
    defPaths << QLatin1String("/usr/local/bin");
    defPaths << QLatin1String("/bin");

#endif

    return defPaths;
}

QDebug operator<<(QDebug dbg, const MetaEngineSettingsContainer& inf)
{
    dbg.nospace() << "[MetaEngineSettingsContainer] exifRotate("
                  << inf.exifRotate << "), ";
    dbg.nospace() << "exifSetOrientation("
                  << inf.exifSetOrientation << "), ";
    dbg.nospace() << "saveComments("
                  << inf.saveComments << "), ";
    dbg.nospace() << "saveDateTime("
                  << inf.saveDateTime << "), ";
    dbg.nospace() << "savePickLabel("
                  << inf.saveColorLabel << "), ";
    dbg.nospace() << "saveColorLabel("
                  << inf.savePickLabel << "), ";
    dbg.nospace() << "saveRating("
                  << inf.saveRating << "), ";
    dbg.nospace() << "saveTemplate("
                  << inf.saveTemplate << "), ";
    dbg.nospace() << "saveTags("
                  << inf.saveTags << "), ";
    dbg.nospace() << "saveFaceTags("
                  << inf.saveFaceTags << "), ";
    dbg.nospace() << "savePosition("
                  << inf.savePosition << "), ";
    dbg.nospace() << "readWithExifTool("
                  << inf.readWithExifTool << "), ";
    dbg.nospace() << "writeWithExifTool("
                  << inf.writeWithExifTool << "), ";
    dbg.nospace() << "writeRawFiles("
                  << inf.writeRawFiles << "), ";
    dbg.nospace() << "writeDngFiles("
                  << inf.writeDngFiles << "), ";
    dbg.nospace() << "updateFileTimeStamp("
                  << inf.updateFileTimeStamp << "), ";
    dbg.nospace() << "rescanImageIfModified("
                  << inf.rescanImageIfModified << "), ";
    dbg.nospace() << "useXMPSidecar4Reading("
                  << inf.useXMPSidecar4Reading << "), ";
    dbg.nospace() << "useCompatibleFileName("
                  << inf.useCompatibleFileName << "), ";
    dbg.nospace() << "useLazySync("
                  << inf.useLazySync << "), ";
    dbg.nospace() << "metadataWritingMode("
                  << inf.metadataWritingMode << "), ";
    dbg.nospace() << "rotationBehavior("
                  << inf.rotationBehavior << "), ";
    dbg.nospace() << "sidecarExtensions("
                  << inf.sidecarExtensions << ")";

    return dbg.space();
}

} // namespace Digikam
