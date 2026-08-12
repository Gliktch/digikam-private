/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "libsinfodlg.h"
#include "digikam_config.h"

// Qt includes

#include <QMap>
#include <QStringList>
#include <QString>
#include <QTreeWidget>
#include <QHeaderView>
#include <QThread>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QtWebEngineWidgetsVersion>

// KDE includes

#include <klocalizedstring.h>
#include <kmemoryinfo.h>
#include <kxmlgui_version.h>

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"
#include "greycstorationfilter.h"
#include "pgfutils.h"
#include "digikam-lcms.h"
#include "metaengine.h"
#include "exiftoolparser.h"
#include "loadingcache.h"
#include "itempropertiestab.h"

#ifdef HAVE_JXL
#   include "dngwriter.h"
#endif

#ifdef HAVE_LENSFUN
#   include "lensfuniface.h"
#endif

#ifdef HAVE_GEOLOCATION
#   include "backendmarble.h"
#endif

#ifdef HAVE_IMAGE_MAGICK

// Pragma directives to reduce warnings from ImageMagick header files.

#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wignored-qualifiers"
#       pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wignored-qualifiers"
#       pragma clang diagnostic ignored "-Wkeyword-macro"
#       pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#   endif

#   include <Magick++.h>

// Restore warnings

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#endif

// C ANSI includes

#ifndef Q_OS_WIN
extern "C"
{
#endif

#ifdef HAVE_JASPER

// Pragma directives to reduce warnings from libjasper header files.

#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wcpp"
#       pragma GCC diagnostic ignored "-Wundef"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wunknown-warning-option"
#       pragma clang diagnostic ignored "-Wpedantic"
#       pragma clang diagnostic ignored "-Wshift-negative-value"
#       pragma clang diagnostic ignored "-Werror"
#       pragma clang diagnostic ignored "-Wundef"
#       pragma clang diagnostic ignored "-Wsign-compare"
#       pragma clang diagnostic ignored "-W#warnings"
#   endif

#   include <jasper/jas_version.h>

// Restore warnings

#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif

#include <png.h>
#include <tiffvers.h>

#ifdef HAVE_HEIF
#   include <libheif/heif_version.h>
#endif

// Pragma directives to reduce warnings from libx265 header files.

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wundef"
#   pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#   pragma clang diagnostic ignored "-Wnested-anon-types"
#   pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#ifdef HAVE_X265
#   include <x265.h>
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Avoid Warnings under Win32

#undef HAVE_STDLIB_H
#undef HAVE_STDDEF_H
#include <jpeglib.h>

#ifndef Q_OS_WIN
}
#endif

#include "digikam_opencv.h"
#include "digikam_version.h"

// NOTE: defined in OpenCV core/private.hpp.

namespace cv
{

const char* currentParallelFramework();

}

namespace cv::ocl
{
    class Device;
}

namespace Digikam
{

class Q_DECL_HIDDEN LibsInfoDlg::Private
{
public:

    static const char* CONTEXT;

public:

    Private() = default;

public:

    QString supportedNo()                           const;
    QString supportedYes()                          const;
    QString checkTriState(int value)                const;
    QString openCVBytesToStringRepr(size_t value)   const;
    QString openCVGetDeviceTypeString(const cv::ocl::Device& device);
};

} // namespace Digikam
