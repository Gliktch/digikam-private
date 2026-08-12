/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-01-06
 * Description : Widget to choose options for autotags scanning
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "autotagsscanwidget.h"

// Qt includes

#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRadioButton>
#include <QToolButton>
#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QList>
#include <QStandardItemModel>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "albummodel.h"
#include "albumselectors.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "squeezedcombobox.h"
#include "dexpanderbox.h"
#include "localizeselector.h"
#include "dnuminput.h"

namespace Digikam
{

class Q_DECL_HIDDEN AutotagsScanWidget::Private
{
public:

    Private() = default;

public:

    AlbumSelectors*   albumSelectors                    = nullptr;

    // ---

    QWidget*          settingsTab                       = nullptr;

    SqueezedComboBox* scanMode                          = nullptr;
    SqueezedComboBox* tagMode                           = nullptr;
    SqueezedComboBox* objectDetectModel                 = nullptr;
    DIntNumInput*     accuracyInput                     = nullptr;
    QCheckBox*        useFullCpuButton                  = nullptr;

    // ---

    LocalizeSelectorList*     trSelectorList            = nullptr;

    // ---

    const QString     configName                        = QLatin1String("Autotags Settings");

    bool              settingsConflicted                = false;

    AutotagsScanWidget::SettingsDisplayMode displayMode = AutotagsScanWidget::SettingsDisplayMode::Normal;
};

} // namespace Digikam
