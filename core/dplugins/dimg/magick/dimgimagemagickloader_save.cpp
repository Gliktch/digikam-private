/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : Magick loader for DImg framework - save method.
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

bool DImgImageMagickLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    // Saving is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    if (observer)
    {
        observer->progressInfo(0.5F);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Try to save image with ImageMagick codecs";

    try
    {
        QVariant formatAttr = imageGetAttribute(QLatin1String("format"));
        QByteArray format   = formatAttr.toByteArray();

        if (observer)
        {
            observer->progressInfo(0.8F);
        }

        Blob pixelBlob(imageData(), imageNumBytes());

        Image image;
        image.size(Geometry(imageWidth(), imageHeight()));
        image.magick("BGRA");
        image.depth(imageBitsDepth());

#if MagickLibVersion < 0x700

        image.matte(imageHasAlpha());

#else

        image.alpha(imageHasAlpha());

#endif

        image.read(pixelBlob);
        image.magick(format.data());

        QByteArray iccRawProfile = m_image->getIccProfile().data();

        if (!iccRawProfile.isEmpty())
        {
            Blob iccBlob(iccRawProfile.data(), iccRawProfile.size());
            image.iccColorProfile(iccBlob);
        }

        image.write(filePath.toUtf8().constData());

        if (observer)
        {
            observer->progressInfo(1.0F);
        }

        imageSetAttribute(QLatin1String("format"), format.toUpper());

        saveMetadata(filePath);

        return true;
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick exception [" << filePath << "]" << error.what();

        return false;
    }

    return true;
}

} // namespace DigikamImageMagickDImgPlugin
