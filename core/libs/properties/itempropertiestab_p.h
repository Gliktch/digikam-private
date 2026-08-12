/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general item information
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itempropertiestab.h"

// Qt includes

#include <QGridLayout>
#include <QStyle>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QPainter>
#include <QPair>
#include <QVariant>
#include <QApplication>
#include <QCollator>
#include <QIcon>
#include <QLocale>
#include <QTime>
#include <QtMath>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "itempropertiestxtlabel.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesTab::Private
{
public:

    Private()  = default;
    ~Private() = default;

public:

    QGridLayout*       glay5                        = nullptr;
    DTextLabelName*    caption                      = nullptr;
    DTextLabelName*    title                        = nullptr;
    DTextLabelName*    pickLabel                    = nullptr;
    DTextLabelName*    colorLabel                   = nullptr;
    DTextLabelName*    rating                       = nullptr;
    DTextLabelName*    tags                         = nullptr;
    DTextLabelName*    peoples                      = nullptr;
    DTextLabelName*    versioned                    = nullptr;
    DTextLabelName*    grouped                      = nullptr;

    DTextLabelValue*   labelFile                    = nullptr;
    // Special case for the file path. See bug #512081
    QLabel*            labelFolder                  = nullptr;
    DTextLabelValue*   labelSymlink                 = nullptr;
    DTextLabelValue*   labelFileModifiedDate        = nullptr;
    DTextLabelValue*   labelFileSize                = nullptr;
    DTextLabelValue*   labelFileOwner               = nullptr;
    DTextLabelValue*   labelFilePermissions         = nullptr;

    DTextLabelValue*   labelImageMime               = nullptr;
    DTextLabelValue*   labelImageDimensions         = nullptr;
    DTextLabelValue*   labelImageRatio              = nullptr;
    DTextLabelValue*   labelImageBitDepth           = nullptr;
    DTextLabelValue*   labelImageColorMode          = nullptr;
    DTextLabelValue*   labelHasSidecar              = nullptr;
    DTextLabelValue*   labelHasGPSInfo              = nullptr;
    DTextLabelValue*   labelVersionedInfo           = nullptr;
    DTextLabelValue*   labelGroupedInfo             = nullptr;

    DTextLabelValue*   labelPhotoMake               = nullptr;
    DTextLabelValue*   labelPhotoModel              = nullptr;
    DTextLabelValue*   labelPhotoDateTime           = nullptr;
    DTextLabelValue*   labelPhotoLens               = nullptr;
    DTextLabelValue*   labelPhotoAperture           = nullptr;
    DTextLabelValue*   labelPhotoFocalLength        = nullptr;
    DTextLabelValue*   labelPhotoExposureTime       = nullptr;
    DTextLabelValue*   labelPhotoSensitivity        = nullptr;
    DTextLabelValue*   labelPhotoExposureMode       = nullptr;
    DTextLabelValue*   labelPhotoFlash              = nullptr;
    DTextLabelValue*   labelPhotoWhiteBalance       = nullptr;

    DTextLabelValue*   labelVideoAspectRatio        = nullptr;
    DTextLabelValue*   labelVideoDuration           = nullptr;
    DTextLabelValue*   labelVideoFrameRate          = nullptr;
    DTextLabelValue*   labelVideoVideoCodec         = nullptr;
    DTextLabelValue*   labelVideoAudioBitRate       = nullptr;
    DTextLabelValue*   labelVideoAudioChannelType   = nullptr;
    DTextLabelValue*   labelVideoAudioCodec         = nullptr;

    // NOTE: special case for the digiKam text properties. See bug #460134
    QLabel*            labelCaption                 = nullptr;
    QLabel*            labelTitle                   = nullptr;
    QLabel*            labelTags                    = nullptr;
    QLabel*            labelPeoples                 = nullptr;
    QLabel*            labelLocation                = nullptr;
    QLabel*            labelRights                  = nullptr;

    QLabel*            labelPick                    = nullptr;
    QLabel*            labelColor                   = nullptr;
    QLabel*            labelRating                  = nullptr;

    DTextLabelValue*   labelPickLabel               = nullptr;
    DTextLabelValue*   labelColorLabel              = nullptr;
};

} // namespace Digikam
