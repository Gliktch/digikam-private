/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpgfloader.h"

// C Ansi includes

extern "C"
{

#ifndef Q_CC_MSVC
#   include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

}

// C++ includes

#include <iostream>
#include <cmath>
#include <cstdio>

// Qt includes

#include <QFile>
#include <QVariant>
#include <QByteArray>
#include <qplatformdefs.h>

// Windows includes

#ifdef Q_OS_WIN32           // krazy:exclude=cpp
#   include <windows.h>
#endif

// Libpgf includes

// Pragma directives to reduce warnings from Libpgf header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#   pragma clang diagnostic ignored "-Wreorder-ctor"
#endif

#include "PGFimage.h"

// Restore warnings
#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"
#include "pgfutils.h"
#include "metaengine.h"

namespace Digikam
{

DImgPGFLoader::DImgPGFLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool DImgPGFLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgPGFLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgPGFLoader::progressCallback(double percent, bool escapeAllowed)
{
    if (m_observer)
    {
        m_observer->progressInfo((float)percent);

        if (escapeAllowed)
        {
            return (!m_observer->continueQuery());
        }
    }

    return false;
}

bool DImgPGFLoader::isReadOnly() const
{
    return false;
}

bool DImgPGFLoader::CallbackForLibPGF(double percent, bool escapeAllowed, void* data)
{
    if (data)
    {
        DImgPGFLoader* const d = static_cast<DImgPGFLoader*>(data);

        return (d->progressCallback(percent, escapeAllowed));
    }

    return false;
}

} // namespace Digikam
