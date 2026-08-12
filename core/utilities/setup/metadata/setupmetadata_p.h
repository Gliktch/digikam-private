/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Ralf Holzer <ralf at well dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setupmetadata.h"

// Qt includes

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QRadioButton>
#include <QStandardPaths>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "advancedmetadatatab.h"
#include "applicationsettings.h"
#include "dactivelabel.h"
#include "digikam_config.h"
#include "digikam_debug.h"
#include "metaengine.h"
#include "metadatapanel.h"
#include "metaenginesettings.h"
#include "exiftoolconfpanel.h"
#include "dexpanderbox.h"
#include "dlayoutbox.h"
#include "drawfiles.h"
#include "setup.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupMetadata::Private
{
public:

    Private() = default;

    void readSettings();

public:

    int                  extensionsMsgBoxResult     = QMessageBox::Help;

    bool                 exifAutoRotateOriginal     = false;
    bool                 exifAutoRotateShowedInfo   = false;
    bool                 sidecarFileNameShowedInfo  = false;

    QGroupBox*           fieldsGroup                = nullptr;
    QGroupBox*           readWriteGroup             = nullptr;
    QGroupBox*           rotationGroup              = nullptr;
    QGroupBox*           rotationAdvGroup           = nullptr;

    QCheckBox*           saveTagsBox                = nullptr;
    QCheckBox*           saveCommentsBox            = nullptr;
    QCheckBox*           saveRatingBox              = nullptr;
    QCheckBox*           savePickLabelBox           = nullptr;
    QCheckBox*           saveColorLabelBox          = nullptr;
    QCheckBox*           saveDateTimeBox            = nullptr;
    QCheckBox*           saveTemplateBox            = nullptr;
    QCheckBox*           saveFaceTags               = nullptr;
    QCheckBox*           savePosition               = nullptr;

    QCheckBox*           useLazySync                = nullptr;
    QCheckBox*           readWithExifToolBox        = nullptr;
    QCheckBox*           writeWithExifToolBox       = nullptr;
    QCheckBox*           writeDngFilesBox           = nullptr;
    QCheckBox*           writeRawFilesBox           = nullptr;
    QLabel*              writeWithExifToolLabel     = nullptr;
    QCheckBox*           writeXMPSidecarBox         = nullptr;
    QCheckBox*           readXMPSidecarBox          = nullptr;
    QCheckBox*           sidecarFileNameBox         = nullptr;
    QCheckBox*           updateFileTimeStampBox     = nullptr;
    QCheckBox*           rescanImageIfModifiedBox   = nullptr;
    QComboBox*           writingModeCombo           = nullptr;

    QRadioButton*        rotateByFlag               = nullptr;
    QRadioButton*        rotateByContents           = nullptr;
    QCheckBox*           allowRotateByMetadata      = nullptr;
    QCheckBox*           allowLossyRotate           = nullptr;
    QCheckBox*           exifRotateBox              = nullptr;
    QCheckBox*           exifSetOrientationBox      = nullptr;

    QCheckBox*           saveToBalooBox             = nullptr;
    QCheckBox*           readFromBalooBox           = nullptr;

    QTabWidget*          tab                        = nullptr;
    QTabWidget*          displaySubTab              = nullptr;

    MetadataPanel*       tagsCfgPanel               = nullptr;
    AdvancedMetadataTab* advTab                     = nullptr;

    ExifToolConfPanel*   exifToolView               = nullptr;

    QLineEdit*           extensionsEdit             = nullptr;
};

} // namespace Digikam
