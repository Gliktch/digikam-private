/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setupalbumview.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QComboBox>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "applicationsettings.h"
#include "dfontselect.h"
#include "dexpanderbox.h"
#include "fullscreensettings.h"
#include "dxmlguiwindow.h"
#include "previewsettings.h"
#include "previewosdsettings.h"
#include "previewosdwidget.h"
#include "setupcategory.h"
#include "setupmime.h"
#include "magnifieritem.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupAlbumView::Private
{
public:

    Private() = default;

public:

    bool                useLargeThumbsOriginal          = false;
    bool                useLargeThumbsShowedInfo        = false;

    QCheckBox*          iconShowNameBox                 = nullptr;
    QCheckBox*          iconShowSizeBox                 = nullptr;
    QCheckBox*          iconShowDateBox                 = nullptr;
    QCheckBox*          iconShowModDateBox              = nullptr;
    QCheckBox*          iconShowResolutionBox           = nullptr;
    QCheckBox*          iconShowAspectRatioBox          = nullptr;
    QCheckBox*          iconShowTitleBox                = nullptr;
    QCheckBox*          iconShowCommentsBox             = nullptr;
    QCheckBox*          iconShowTagsBox                 = nullptr;
    QCheckBox*          iconShowOverlaysBox             = nullptr;
    QCheckBox*          iconShowFullscreenBox           = nullptr;
    QCheckBox*          iconShowRatingBox               = nullptr;
    QCheckBox*          iconShowPickLabelBox            = nullptr;
    QCheckBox*          iconShowColorLabelBox           = nullptr;
    QCheckBox*          iconShowFormatBox               = nullptr;
    QCheckBox*          iconShowCoordinatesBox          = nullptr;
    QRadioButton*       previewFastPreview              = nullptr;
    QRadioButton*       previewFullView                 = nullptr;
    QComboBox*          previewRawMode                  = nullptr;
    QComboBox*          magnifierZoomFactor             = nullptr;
    QComboBox*          magnifierSize                   = nullptr;
    QCheckBox*          previewConvertToEightBit        = nullptr;
    QCheckBox*          previewScaleFitToWindow         = nullptr;
    QCheckBox*          previewSmoothScaled             = nullptr;
    QCheckBox*          previewShowIcons                = nullptr;
    QCheckBox*          previewAutoPlay                 = nullptr;
    QCheckBox*          previewOverlay                  = nullptr;
    QCheckBox*          showFolderTreeViewItemsCount    = nullptr;
    QCheckBox*          largeThumbsBox                  = nullptr;

    QComboBox*          iconTreeThumbSize               = nullptr;
    QComboBox*          iconTreeFaceSize                = nullptr;
    QComboBox*          leftClickActionComboBox         = nullptr;

    QTabWidget*         tab                             = nullptr;

    DFontSelect*        iconViewFontSelect              = nullptr;
    DFontSelect*        treeViewFontSelect              = nullptr;

    FullScreenSettings* fullScreenSettings              = nullptr;

    PreviewOsdSettings  osdSettings;
    PreviewOsdWidget*   osdWidget                       = nullptr;

    SetupCategory*      category                        = nullptr;
    SetupMime*          mimetype                        = nullptr;
    const int           spacing                         = layoutSpacing();
};

} // namespace Digikam
