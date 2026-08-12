/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-14
 * Description : A JPEG-2000 IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dimgjpeg2000loader.h"

// C++ includes

#include <memory>

// Qt includes

#include <QFile>
#include <QByteArray>
#include <QTextStream>
#include <QScopedPointer>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "dmetadata.h"

// Jasper includes

#ifndef Q_CC_MSVC
extern "C"
{
#endif

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wcpp"
#   pragma GCC diagnostic ignored "-Wundef"
#endif

#if defined(Q_OS_DARWIN) && defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wpedantic"
#   pragma clang diagnostic ignored "-Wshift-negative-value"
#   pragma clang diagnostic ignored "-Werror"
#   pragma clang diagnostic ignored "-Wundef"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-W#warnings"
#endif

#include <jasper/jasper.h>
#include <jasper/jas_version.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_OS_DARWIN) && defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#ifndef Q_CC_MSVC
}
#endif

