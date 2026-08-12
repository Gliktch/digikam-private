/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "thumbnailcreator.h"

// Qt includes

#include <QUrl>
#include <QFile>
#include <QBuffer>
#include <QPainter>
#include <QIODevice>
#include <QFileInfo>
#include <QUrlQuery>
#include <QApplication>
#include <QMimeDatabase>
#include <QTemporaryFile>
#include <QScopedPointer>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "metaengine_previews.h"
#include "metaengine_rotation.h"
#include "filereadwritelock.h"
#include "drawdecoder.h"
#include "drawfiles.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "dimgloader.h"
#include "dimgloaderobserver.h"
#include "dmetadata.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "loadsavethread.h"
#include "jpegutils.h"
#include "pgfutils.h"
#include "tagregion.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"
#include "thumbsdbbackend.h"
#include "thumbnailsize.h"

#ifdef HAVE_MEDIAPLAYER
#   include "videothumbnailer.h"
#   include "videostripfilter.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN ThumbnailImage
{
public:

    ThumbnailImage() = default;

    bool isNull() const
    {
        return qimage.isNull();
    }

public:

    QImage qimage;
    int    exifOrientation  = DMetadata::ORIENTATION_UNSPECIFIED;
};

// -------------------------------------------------------------------

class Q_DECL_HIDDEN ThumbnailCreator::Private
{
public:

    Private()
    {
        fastRawSettings.optimizeTimeLoading();
        fastRawSettings.rawPrm.halfSizeColorImage = true;
        fastRawSettings.rawPrm.sixteenBitsImage   = false;
    }

    int storageSize() const;

public:

    bool                            exifRotate          = true;
    bool                            removeAlphaChannel  = true;
    bool                            onlyLargeThumbnails = false;

    ThumbnailCreator::StorageMethod thumbnailStorage    = ThumbnailCreator::FreeDesktopStandard;
    ThumbnailInfoProvider*          infoProvider        = nullptr;
    int                             dbIdForReplacement  = -1;

    int                             thumbnailSize       = 0;

    QString                         error;
    QString                         bigThumbPath;
    QString                         smallThumbPath;

    // Used internally as PNG metadata. Do not use i18n.
    QString                         digiKamFingerPrint  = QLatin1String("Digikam Thumbnail Generator");

    QImage                          alphaImage;

    DImgLoaderObserver*             observer            = nullptr;
    DRawDecoding                    rawSettings;
    DRawDecoding                    fastRawSettings;
};

} // namespace Digikam
