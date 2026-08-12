/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - private containers.
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itemscanner.h"

// Qt includes

#include <QImageReader>
#include <QElapsedTimer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "coredburl.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "coredboperationgroup.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "facetagseditor.h"
#include "itemcomments.h"
#include "itemcopyright.h"
#include "itemextendedproperties.h"
#include "itemhistorygraph.h"
#include "metaenginesettings.h"
#include "tagregion.h"
#include "tagscache.h"
#include "iostream"
#include "dimagehistory.h"
#include "itemhistorygraphdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemScannerCommit
{

public:

    enum Operation
    {
        NoOp,
        AddItem,
        UpdateItem
    };

public:

    ItemScannerCommit() = default;

public:

    Operation                        operation              = NoOp;

    qlonglong                        copyImageAttributesId  = -1;

    bool                             commitItemInformation  = false;
    bool                             commitImageMetadata    = false;
    bool                             commitVideoMetadata    = false;
    bool                             commitItemPosition     = false;
    bool                             commitItemComments     = false;
    bool                             commitItemCopyright    = false;
    bool                             commitFaces            = false;
    bool                             commitIPTCCore         = false;
    bool                             hasColorTag            = false;
    bool                             hasPickTag             = false;

    DatabaseFields::ItemInformation  imageInformationFields;
    QVariantList                     imageInformationInfos;

    QVariantList                     imageMetadataInfos;
    QVariantList                     videoMetadataInfos;
    QVariantList                     imagePositionInfos;

    QStringList                      filteredKeywords;

    CaptionsMap                      captions;
    QString                          headline;
    CaptionsMap                      titles;

    Template                         copyrightTemplate;
    QMultiMap<QString, QVariant>     metadataFacesMap;

    QVariantList                     iptcCoreMetadataInfos;

    QList<int>                       tagIds;
    QString                          historyXml;
    QString                          uuid;
};

// ---------------------------------------------------------------------------------------

class Q_DECL_HIDDEN LessThanByProximityToSubject
{
public:

    explicit LessThanByProximityToSubject(const ItemInfo& subject);

    bool operator()(const ItemInfo& a, const ItemInfo& b);

public:

    ItemInfo subject;
};

// ---------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemScanner::Private
{
public:

    Private();
    ~Private();

public:

    bool                        hasImage             = false;
    bool                        hasMetadata          = false;
    bool                        loadedFromDisk       = false;

    QFileInfo                   fileInfo;

    DMetadata*                  metadata             = nullptr;
    DImg                        img;
    ItemScanInfo                scanInfo;
    ItemScanner::ScanMode       scanMode             = ModifiedScan;
    MetaEngineSettingsContainer settings             = MetaEngineSettings::instance()->settings();

    bool                        hasHistoryToResolve  = false;

    ItemScannerCommit            commit;

    QElapsedTimer                timer;
};

} // namespace Digikam
