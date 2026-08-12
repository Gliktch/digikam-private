/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : Magick loader for DImg framework.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2023 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgimagemagickloader.h"

// ImageMagick includes

// Pragma directives to reduce warnings from ImageMagick header files.
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wignored-qualifiers"
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wignored-qualifiers"
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#include <Magick++.h>

// Restore warnings

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

using namespace Magick;

namespace DigikamImageMagickDImgPlugin
{

DImgImageMagickLoader::DImgImageMagickLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool DImgImageMagickLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgImageMagickLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgImageMagickLoader::isReadOnly() const
{
    return false;
}

} // namespace DigikamImageMagickDImgPlugin
