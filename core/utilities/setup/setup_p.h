/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setup.h"

// Qt includes

#include <QPointer>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"
#include "loadingcacheinterface.h"
#include "applicationsettings.h"
#include "setupalbumview.h"
#include "setupcamera.h"
#include "setupcollections.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setuplighttable.h"
#include "setupsurvey.h"
#include "setupmetadata.h"
#include "setupmisc.h"
#include "setuptooltip.h"
#include "setupdatabase.h"
#include "setupplugins.h"
#include "importsettings.h"
#include "onlineversiondlg.h"

#ifdef HAVE_GEOLOCATION
#   include "setupgeolocation.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN Setup::Private
{
public:

    Private() = default;

public:

    DConfigDlgWdgItem*       page_database              = nullptr;
    DConfigDlgWdgItem*       page_collections           = nullptr;
    DConfigDlgWdgItem*       page_albumView             = nullptr;
    DConfigDlgWdgItem*       page_tooltip               = nullptr;
    DConfigDlgWdgItem*       page_metadata              = nullptr;
    DConfigDlgWdgItem*       page_template              = nullptr;
    DConfigDlgWdgItem*       page_lighttable            = nullptr;
    DConfigDlgWdgItem*       page_survey                = nullptr;

#ifdef HAVE_GEOLOCATION

    DConfigDlgWdgItem*       page_geolocation           = nullptr;

#endif

    DConfigDlgWdgItem*       page_editor                = nullptr;
    DConfigDlgWdgItem*       page_icc                   = nullptr;
    DConfigDlgWdgItem*       page_camera                = nullptr;
    DConfigDlgWdgItem*       page_plugins               = nullptr;
    DConfigDlgWdgItem*       page_misc                  = nullptr;

    SetupDatabase*           databasePage               = nullptr;
    SetupCollections*        collectionsPage            = nullptr;
    SetupAlbumView*          albumViewPage              = nullptr;
    SetupToolTip*            tooltipPage                = nullptr;
    SetupMetadata*           metadataPage               = nullptr;
    SetupTemplate*           templatePage               = nullptr;
    SetupLightTable*         lighttablePage             = nullptr;
    SetupSurvey*             surveyPage                 = nullptr;

#ifdef HAVE_GEOLOCATION

    SetupGeolocation*        geolocationPage            = nullptr;

#endif

    SetupEditor*             editorPage                 = nullptr;
    SetupICC*                iccPage                    = nullptr;
    SetupCamera*             cameraPage                 = nullptr;
    SetupPlugins*            pluginsPage                = nullptr;
    SetupMisc*               miscPage                   = nullptr;

public:

    DConfigDlgWdgItem* pageItem(Setup::Page page) const;
};

} // namespace Digikam
