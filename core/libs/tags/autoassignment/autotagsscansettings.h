/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Autotags scan settings container.
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDebug>

// Local includes

#include "album.h"
#include "iteminfo.h"
#include "digikam_export.h"
#include "dnnmodeldefinitions.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AutotagsScanSettings
{
    Q_GADGET

public:

    /**
     * @brief Different possible mode processed while scanning operation.
     */
    enum ScanMode
    {
        AllItems = 0,        ///< Scan all items
        NonAssignedItems     ///< Scan only items with no tags assigned.
    };
    Q_ENUM(ScanMode)

    /**
     * @brief Different possible mode processed with tags while scanning operation.
     */
    enum TagMode
    {
        Replace = 0,        ///< Clean all tags already assigned
        Update              ///< Add new tags to existing tags
    };
    Q_ENUM(TagMode)

public:

    AutotagsScanSettings()  = default;
    AutotagsScanSettings(const AutotagsScanSettings& other);
    ~AutotagsScanSettings() = default;

    AutotagsScanSettings& operator=(const AutotagsScanSettings& other);

public:

    void readFromConfig();
    void readFromConfig(const KConfigGroup&);
    void writeToConfig();
    void writeToConfig(KConfigGroup&);

public:

    /// Scanning mode.
    ScanMode                                scanMode                    = ScanMode::AllItems;

    /// Processing power.
    TagMode                                 tagMode                     = TagMode::Replace;

    /// Processing using all CPU available.
    bool                                    useFullCpu                  = false;

    /// Object Detection Model.
    QString                                 objectDetectModel           = QStringLiteral("yolov11-nano");

    /// Confidence threshold
    int                                     uiConfidenceThreshold       = 7;

    /// Autotags languages
    QStringList                             languages;

    /// Whole albums checked.
    bool                                    wholeAlbums                 = true;

    /// Albums to scan.
    AlbumList                               albums;

    /// Set true for BQM
    bool                                    bqmMode                     = false;

private:

    const QString                           configName                  = QLatin1String("Autotags Settings");
    const QString                           configScanMode              = QLatin1String("Autotags Scan Mode");
    const QString                           configTagMode               = QLatin1String("Autotags Tag Mode");
    const QString                           configUseFullCpu            = QLatin1String("Autotags Use Full CPU");
    const QString                           configObjectDetectModel     = QLatin1String("Autotags Object Detection Model");
    const QString                           configObjectDetectAccuracy  = QLatin1String("Autotags Object Detection Accuracy");
    const QString                           configLanguages             = QLatin1String("Autotags Languages");
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
DIGIKAM_GUI_EXPORT QDebug operator<<(QDebug dbg, const AutotagsScanSettings& s);

} // namespace Digikam
