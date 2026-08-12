/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "setuptooltip.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "applicationsettings.h"
#include "importsettings.h"
#include "setupcamera.h"
#include "dfontselect.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupToolTip::Private
{
public:

    Private() = default;

public:

    const int    spacing                    = layoutSpacing();

    bool         cameraUseFileMetadata      = false;

    QWidget*     panel                      = nullptr;
    QCheckBox*   showToolTipsBox            = nullptr;

    QCheckBox*   showFileNameBox            = nullptr;
    QCheckBox*   showFileDateBox            = nullptr;
    QCheckBox*   showFileSizeBox            = nullptr;
    QCheckBox*   showImageTypeBox           = nullptr;
    QCheckBox*   showImageDimBox            = nullptr;
    QCheckBox*   showImageARBox             = nullptr;

    QCheckBox*   showPhotoMakeBox           = nullptr;
    QCheckBox*   showPhotoLensBox           = nullptr;
    QCheckBox*   showPhotoDateBox           = nullptr;
    QCheckBox*   showPhotoFocalBox          = nullptr;
    QCheckBox*   showPhotoExpoBox           = nullptr;
    QCheckBox*   showPhotoModeBox           = nullptr;
    QCheckBox*   showPhotoFlashBox          = nullptr;
    QCheckBox*   showPhotoWbBox             = nullptr;

    QCheckBox*   showVideoAspectRatio       = nullptr;
    QCheckBox*   showVideoAudioBitRate      = nullptr;
    QCheckBox*   showVideoAudioChannelType  = nullptr;
    QCheckBox*   showVideoAudioCodec        = nullptr;
    QCheckBox*   showVideoDuration          = nullptr;
    QCheckBox*   showVideoFrameRate         = nullptr;
    QCheckBox*   showVideoVideoCodec        = nullptr;

    QCheckBox*   showAlbumNameBox           = nullptr;
    QCheckBox*   showTitlesBox              = nullptr;
    QCheckBox*   showCommentsBox            = nullptr;
    QCheckBox*   showTagsBox                = nullptr;
    QCheckBox*   showLabelsBox              = nullptr;

    QCheckBox*   showAlbumToolTipsBox       = nullptr;
    QCheckBox*   showAlbumTitleBox          = nullptr;
    QCheckBox*   showAlbumDateBox           = nullptr;
    QCheckBox*   showAlbumCollectionBox     = nullptr;
    QCheckBox*   showAlbumCategoryBox       = nullptr;
    QCheckBox*   showAlbumCaptionBox        = nullptr;
    QCheckBox*   showAlbumPreviewBox        = nullptr;

    QCheckBox*   showImportToolTipsBox      = nullptr;
    QCheckBox*   showItemTitleBox           = nullptr;
    QCheckBox*   showItemDateBox            = nullptr;
    QCheckBox*   showItemSizeBox            = nullptr;
    QCheckBox*   showItemTypeBox            = nullptr;
    QCheckBox*   showItemDimensionsBox      = nullptr;
    QCheckBox*   showItemPhotoMakeBox       = nullptr;
    QCheckBox*   showItemPhotoLensBox       = nullptr;
    QCheckBox*   showItemPhotoFocalBox      = nullptr;
    QCheckBox*   showItemPhotoExpoBox       = nullptr;
    QCheckBox*   showItemPhotoFlashBox      = nullptr;
    QCheckBox*   showItemPhotoWBBox         = nullptr;

    QGroupBox*   fileSettingBox             = nullptr;
    QGroupBox*   photoSettingBox            = nullptr;
    QGroupBox*   videoSettingBox            = nullptr;
    QGroupBox*   digikamSettingBox          = nullptr;
    QGroupBox*   albumSettingBox            = nullptr;
    QGroupBox*   importSettingBox           = nullptr;

    QTabWidget*  tab                        = nullptr;

    DFontSelect* fontSelect                 = nullptr;
};

} // namespace Digikam
