/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : a PNG image loader for DImg framework.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#define PNG_BYTES_TO_CHECK 4

#include "dimgpngloader.h"

// C ANSI includes

extern "C"
{
#ifndef Q_CC_MSVC
#   include <unistd.h>
#endif
}

// C++ includes

#include <cstdlib>
#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>
#include <QSysInfo>

// Local includes

#include "metaengine.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"
#include "dimgloaderobserver.h"

// libPNG includes

extern "C"
{
#include <png.h>
}

using namespace Digikam;

namespace DigikamPNGDImgPlugin
{

DImgPNGLoader::DImgPNGLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool DImgPNGLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgPNGLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgPNGLoader::isReadOnly() const
{
    return false;
}

} // namespace DigikamPNGDImgPlugin
