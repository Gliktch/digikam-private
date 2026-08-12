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

#pragma once

// Qt includes

#include <QFlags>
#include <QDebug>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "metaengine.h"

class KConfigGroup;

namespace Digikam
{

/**
 * @brief The class MetaEngineSettingsContainer encapsulates all metadata related settings.
 *
 * @note this allows supply changed arguments to MetadataHub without changing the global settings.
 */
class DIGIKAM_EXPORT MetaEngineSettingsContainer
{
public:

    /**
     * Describes the allowed and desired operation when rotating a picture.
     * The modes are in escalating order and describe if an operation is allowed.
     * What is actually done will be governed by what is possible:
     * 1) RAW files cannot by rotated by content, setting the metadata may be problematic
     * 2) Read-Only files cannot edited, neither content nor metadata
     * 3) Writable files will have lossy compression
     * 4) Only JPEG and PGF offer lossless rotation
     * Using a contents-based rotation always implies resetting the flag.
     */
    enum RotationBehaviorFlag
    {
        NoRotation               = 0,
        RotateByInternalFlag     = 1 << 0,
        RotateByMetadataFlag     = 1 << 1,
        RotateByLosslessRotation = 1 << 2,
        RotateByLossyRotation    = 1 << 3,

        RotatingFlags            = RotateByInternalFlag     | RotateByMetadataFlag,
        RotatingPixels           = RotateByLosslessRotation | RotateByLossyRotation
    };
    Q_DECLARE_FLAGS(RotationBehaviorFlags, RotationBehaviorFlag)

    enum AlbumDateSource
    {
        NewestItemDate = 0,
        OldestItemDate,
        AverageDate,
        FolderDate,
        IgnoreDate
    };

public:

    MetaEngineSettingsContainer()  = default;
    ~MetaEngineSettingsContainer() = default;

public:

    void readFromConfig(const KConfigGroup& group);
    void writeToConfig(KConfigGroup& group)  const;

    bool isMetadataWriteEnabled()            const;

    QStringList defaultExifToolSearchPaths() const;

public:

    bool                            exifRotate              = true;
    bool                            exifSetOrientation      = true;

    bool                            saveComments            = false;
    bool                            saveDateTime            = false;
    bool                            savePickLabel           = false;
    bool                            saveColorLabel          = false;
    bool                            saveRating              = false;

    bool                            saveTemplate            = false;
    bool                            saveTags                = false;
    bool                            saveFaceTags            = false;
    bool                            savePosition            = false;

    bool                            readWithExifTool        = false;
    bool                            writeWithExifTool       = false;
    bool                            writeRawFiles           = false;
    bool                            writeDngFiles           = false;
    bool                            updateFileTimeStamp     = true;
    bool                            rescanImageIfModified   = false;
    bool                            useXMPSidecar4Reading   = false;
    bool                            useCompatibleFileName   = false;
    bool                            useLazySync             = false;
    bool                            useFastScan             = false;

    MetaEngine::MetadataWritingMode metadataWritingMode     = MetaEngine::WRITE_TO_FILE_ONLY;

    RotationBehaviorFlags           rotationBehavior        = RotationBehaviorFlags(RotatingFlags | RotateByLosslessRotation);

    AlbumDateSource                 albumDateFrom           = OldestItemDate;

    QStringList                     sidecarExtensions;

    QString                         exifToolPath;
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const MetaEngineSettingsContainer& inf);

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::MetaEngineSettingsContainer::RotationBehaviorFlags)
