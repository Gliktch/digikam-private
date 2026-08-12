/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "maintenancedlg.h"

// Qt includes

#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QComboBox>
#include <QScrollArea>
#include <QIcon>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "setup.h"
#include "albumselectors.h"
#include "facescansettings.h"
#include "imagequalitysettings.h"
#include "imagequalitywidget.h"
#include "metadatasynchronizer.h"
#include "dxmlguiwindow.h"
#include "applicationsettings.h"
#include "drangebox.h"
#include "localizeselector.h"
#include "autotagsscanwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN MaintenanceDlg::Private
{
public:

    enum Operation
    {
        Options = 0,
        NewItems,
        DbCleanup,
        Thumbnails,
        FingerPrints,
        Duplicates,
        FaceManagement,
        AutotagsAssignment,
        ImageQualitySorter,
        MetadataSync,
        Stretch
    };

public:

    Private() = default;

public:

    const QString configGroupName                       = QLatin1String("MaintenanceDlg Settings");
    const QString configUseLastSettings                 = QLatin1String("UseLastSettings");
    const QString configUseMutiCoreCPU                  = QLatin1String("UseMutiCoreCPU");
    const QString configNewItems                        = QLatin1String("NewItems");
    const QString configThumbnails                      = QLatin1String("Thumbnails");
    const QString configScanThumbs                      = QLatin1String("ScanThumbs");
    const QString configFingerPrints                    = QLatin1String("FingerPrints");
    const QString configScanFingerPrints                = QLatin1String("ScanFingerPrints");
    const QString configDuplicates                      = QLatin1String("Duplicates");
    const QString configMinSimilarity                   = QLatin1String("minSimilarity");
    const QString configMaxSimilarity                   = QLatin1String("maxSimilarity");
    const QString configDuplicatesRestriction           = QLatin1String("duplicatesRestriction");
    const QString configFaceManagement                  = QLatin1String("FaceManagement");
    const QString configAutotagsAssignment              = QLatin1String("AutotagsAssignment");
    const QString configImageQualitySorter              = QLatin1String("ImageQualitySorter");
    const QString configQualityScanMode                 = QLatin1String("QualityScanMode");
    const QString configQualitySettingsSelected         = QLatin1String("QualitySettingsSelected");
    const QString configMetadataSync                    = QLatin1String("MetadataSync");
    const QString configCleanupDatabase                 = QLatin1String("CleanupDatabase");
    const QString configCleanupThumbDatabase            = QLatin1String("CleanupThumbDatabase");
    const QString configCleanupFacesDatabase            = QLatin1String("CleanupFacesDatabase");
    const QString configCleanupSimilarityDatabase       = QLatin1String("CleanupSimilarityDatabase");
    const QString configShrinkDatabases                 = QLatin1String("ShrinkDatabases");
    const QString configSyncDirection                   = QLatin1String("SyncDirection");

public:

    QDialogButtonBox*         buttons                   = nullptr;
    QLabel*                   logo                      = nullptr;
    QLabel*                   title                     = nullptr;
    QCheckBox*                scanThumbs                = nullptr;
    QCheckBox*                scanFingerPrints          = nullptr;
    QCheckBox*                useLastSettings           = nullptr;
    QCheckBox*                useMutiCoreCPU            = nullptr;
    QCheckBox*                cleanThumbsDb             = nullptr;
    QCheckBox*                cleanFacesDb              = nullptr;
    QCheckBox*                cleanSimilarityDb         = nullptr;
    QCheckBox*                retrainAllFaces           = nullptr;
    QCheckBox*                resetFaceDb               = nullptr;
    QCheckBox*                clearRejectedFaces        = nullptr;
    QCheckBox*                shrinkDatabases           = nullptr;
    QComboBox*                qualityScanMode           = nullptr;
    AutotagsScanWidget*       autotagsWidget            = nullptr;
    QPushButton*              metadataSetup             = nullptr;
    ImageQualityWidget*       qualityWidget             = nullptr;
    QComboBox*                syncDirection             = nullptr;
    DHBox*                    similarityRangeBox        = nullptr;
    DHBox*                    dupeRestrictionBox        = nullptr;
    DVBox*                    vbox                      = nullptr;
    DVBox*                    vbox2                     = nullptr;
    DVBox*                    vbox3                     = nullptr;
    DVBox*                    vbox4                     = nullptr;
    DVBox*                    vbox5                     = nullptr;
    DVBox*                    duplicatesBox             = nullptr;
    DIntRangeBox*             similarityRange           = nullptr;
    QComboBox*                searchResultRestriction   = nullptr;
    DExpanderBox*             expanderBox               = nullptr;
    AlbumSelectors*           albumSelectors            = nullptr;
    LocalizeSelectorList*     trSelectorList            = nullptr;
    DIntNumInput*             autotagsAccuracyInput     = nullptr;
};

} // namespace Digikam
