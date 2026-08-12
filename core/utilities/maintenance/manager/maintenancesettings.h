/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager settings
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDebug>

// Local includes

#include "album.h"
#include "facescansettings.h"
#include "haariface.h"
#include "imagequalitysettings.h"
#include "metadatasynchronizer.h"
#include "imagequalitysorter.h"
#include "autotagsscansettings.h"

namespace Digikam
{

class MaintenanceSettings
{

public:

    MaintenanceSettings()  = default;
    ~MaintenanceSettings() = default;

public:

    bool                                    wholeAlbums                  = true;
    bool                                    wholeTags                    = true;

    AlbumList                               albums;
    AlbumList                               tags;


    /// Use Multi-core CPU to process items.
    bool                                    useMutiCoreCPU               = false;

    /// Find new items on whole collection.
    bool                                    newItems                     = false;

    /// Generate thumbnails
    bool                                    thumbnails                   = false;

    /// Rebuild all thumbnails or only scan missing items.
    bool                                    scanThumbs                   = false;

    /// Generate finger-prints
    bool                                    fingerPrints                 = false;

    /// Rebuild all fingerprints or only scan missing items.
    bool                                    scanFingerPrints             = false;

    /// Scan for new items
    bool                                    duplicates                   = false;

    /// Minimal similarity between items to compare, in percents.
    int                                     minSimilarity                = 90;

    /// Maximal similarity between items to compare, in percents.
    int                                     maxSimilarity                = 100;

    /// The type of restrictions to apply on duplicates search results.
    HaarIface::DuplicatesSearchRestrictions duplicatesRestriction        = HaarIface::DuplicatesSearchRestrictions::None;

    /// Scan for faces.
    bool                                    faceManagement               = false;

    /// Face detection settings.
    FaceScanSettings                        faceSettings;

    /// clear rejected face matches.
    bool                                    clearRejectedFaces           = false;

    /// Autotags assignment.
    bool                                    autotagsAssignment           = false;

    /// Autotags settings.
    AutotagsScanSettings                    autotagsSettings;

    /// Perform Image Quality Sorting.
    bool                                    qualitySort                  = false;

    /// Image Quality Sorting Settings.
    ImageQualitySettings                    qualitySettings;

    /// Sync metadata and DB.
    bool                                    metadataSync                 = false;

    /// Sync direction (image metadata <-> DB).
    int                                     syncDirection                = MetadataSynchronizer::WriteFromDatabaseToFile;

    /// Perform database cleanup
    bool                                    databaseCleanup              = false;
    bool                                    cleanThumbDb                 = false;
    bool                                    cleanFacesDb                 = false;
    bool                                    cleanSimilarityDb            = false;
    bool                                    shrinkDatabases              = false;
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const MaintenanceSettings& s);

} // namespace Digikam
