/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setupmisc.h"

// Qt includes

#include <QApplication>
#include <QDirIterator>
#include <QButtonGroup>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyle>
#include <QStyleFactory>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QMessageBox>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_config.h"
#include "dexpanderbox.h"
#include "dlayoutbox.h"
#include "dfontselect.h"
#include "thememanager.h"
#include "metaenginesettings.h"
#include "applicationsettings.h"
#include "systemsettingswidget.h"
#include "spellcheckconfig.h"
#include "onlineversionchecker.h"
#include "setup.h"
#include "localizeconfig.h"
#include "facebackgroundrecognition.h"
#include "dateformatmodifier.h"

#ifdef HAVE_SONNET
#   include "spellcheckconfig.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN SetupMisc::Private
{
public:

    Private() = default;

public:

    QTabWidget*               tab                                       = nullptr;

    QLabel*                   updateTypeLabel                           = nullptr;
    QCheckBox*                updateWithDebug                           = nullptr;

    QLabel*                   sidebarTypeLabel                          = nullptr;
    QLabel*                   albumDateSourceLabel                      = nullptr;
    QLabel*                   stringComparisonTypeLabel                 = nullptr;
    QLabel*                   applicationStyleLabel                     = nullptr;
    QLabel*                   applicationIconLabel                      = nullptr;
    QLabel*                   dateTimeFormatLabel                       = nullptr;
    QLabel*                   minSimilarityBoundLabel                   = nullptr;

    QLineEdit*                dateTimeFormatEdit                        = nullptr;
    QToolButton*              dateTimeFormatReset                       = nullptr;

    QCheckBox*                showSplashCheck                           = nullptr;
    QCheckBox*                showTrashDeleteDialogCheck                = nullptr;
    QCheckBox*                showPermanentDeleteDialogCheck            = nullptr;
    QCheckBox*                sidebarApplyDirectlyCheck                 = nullptr;
    QCheckBox*                useNativeFileDialogCheck                  = nullptr;
    QCheckBox*                drawFramesToGroupedCheck                  = nullptr;
    QCheckBox*                selectFirstAlbumItemCheck                 = nullptr;
    QCheckBox*                expandNewCurrentItemCheck                 = nullptr;
    QCheckBox*                scrollItemToCenterCheck                   = nullptr;
    QCheckBox*                showOnlyPersonTagsInPeopleSidebarCheck    = nullptr;
    QCheckBox*                scanAtStart                               = nullptr;
    QCheckBox*                useFastScan                               = nullptr;
    QCheckBox*                detectFaces                               = nullptr;
    QCheckBox*                faceRecognitionBackgroundScan             = nullptr;
    QCheckBox*                cleanAtStart                              = nullptr;

    QComboBox*                updateType                                = nullptr;
    QComboBox*                sidebarType                               = nullptr;
    QComboBox*                albumDateSource                           = nullptr;
    QComboBox*                stringComparisonType                      = nullptr;
    QComboBox*                applicationStyle                          = nullptr;
    QComboBox*                applicationIcon                           = nullptr;
    DFontSelect*              applicationFont                           = nullptr;

    QSpinBox*                 minimumSimilarityBound                    = nullptr;

    SystemSettingsWidget*     systemSettingsWidget                      = nullptr;

#ifdef HAVE_SONNET

    SpellCheckConfig*         spellCheckWidget                          = nullptr;

#endif

    LocalizeConfig*           localizeWidget                            = nullptr;
    QHash<int, QButtonGroup*> groupingButtons                           = QHash<int, QButtonGroup*>();

    QWidget*                   parent                                   = nullptr;
    const int                  spacing                                  = layoutSpacing();
};

} // namespace Digikam
