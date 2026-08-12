/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-24
 * Description : Color management setup tab.
 *
 * SPDX-FileCopyrightText: 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setupicc.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDialogButtonBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dlayoutbox.h"
#include "squeezedcombobox.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "applicationsettings.h"
#include "iccprofileinfodlg.h"
#include "iccprofilescombobox.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "dactivelabel.h"
#include "dfileselector.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupICC::Private
{
public:

    Private() = default;

public:

    QLabel*                     iccFolderLabel          = nullptr;

    QCheckBox*                  enableColorManagement   = nullptr;
    QCheckBox*                  defaultSRGBConvert      = nullptr;
    QCheckBox*                  bpcAlgorithm            = nullptr;
    QCheckBox*                  managedView             = nullptr;
    QCheckBox*                  managedPreviews         = nullptr;

    QRadioButton*               defaultAskMismatch      = nullptr;
    QRadioButton*               defaultConvertMismatch  = nullptr;
    QRadioButton*               defaultAskMissing       = nullptr;
    QRadioButton*               defaultSRGBMissing      = nullptr;
    QRadioButton*               defaultWSMissing        = nullptr;
    QRadioButton*               defaultInputMissing     = nullptr;
    QRadioButton*               defaultAskRaw           = nullptr;
    QRadioButton*               defaultInputRaw         = nullptr;
    QRadioButton*               defaultGuessRaw         = nullptr;

    QPushButton*                infoWorkProfiles        = nullptr;
    QPushButton*                infoMonitorProfiles     = nullptr;
    QPushButton*                infoInProfiles          = nullptr;
    QPushButton*                infoProofProfiles       = nullptr;

    QGroupBox*                  workspaceGB             = nullptr;
    QGroupBox*                  mismatchGB              = nullptr;
    QGroupBox*                  missingGB               = nullptr;
    QGroupBox*                  rawGB                   = nullptr;
    QGroupBox*                  inputGB                 = nullptr;
    QGroupBox*                  viewGB                  = nullptr;
    QGroupBox*                  proofGB                 = nullptr;
    QGroupBox*                  iccFolderGB             = nullptr;
    QGroupBox*                  advancedSettingsGB      = nullptr;

    DFileSelector*              defaultPathKU           = nullptr;

    IccRenderingIntentComboBox* renderingIntentKC       = nullptr;

    QWidget*                    behaviorPanel           = nullptr;
    QWidget*                    profilesPanel           = nullptr;
    QWidget*                    advancedPanel           = nullptr;
    QTabWidget*                 tab                     = nullptr;
    QDialogButtonBox*           dlgBtnBox               = nullptr;

    IccProfilesComboBox*        inProfilesKC            = nullptr;
    IccProfilesComboBox*        workProfilesKC          = nullptr;
    IccProfilesComboBox*        proofProfilesKC         = nullptr;
    IccProfilesComboBox*        monitorProfilesKC       = nullptr;
};

} // namespace Digikam
