/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : Magick loader for DImg framework - load method.
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

bool DImgImageMagickLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath);

    // Loading is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    if (observer)
    {
        observer->progressInfo(0.5F);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Try to load image with ImageMagick codecs";

    try
    {
        Image image;

        if (m_loadFlags & LoadImageData)
        {
            try
            {
                image.read(filePath.toUtf8().constData());
            }
            catch (Warning& warning)
            {
                qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick warning [" << filePath << "]" << warning.what();
            }

            if (observer)
            {
                observer->progressInfo(0.8F);
            }

            qCDebug(DIGIKAM_DIMG_LOG) << "IM to DImg      :" << image.columns() << image.rows();
            qCDebug(DIGIKAM_DIMG_LOG) << "IM QuantumRange :" << QuantumRange;
            qCDebug(DIGIKAM_DIMG_LOG) << "IM Depth        :" << image.depth();
            qCDebug(DIGIKAM_DIMG_LOG) << "IM Format       :" << image.format().c_str();

            m_sixteenBit     = (image.depth() == 16);
            StorageType type = m_sixteenBit ? ShortPixel : CharPixel;
            uchar* data      = new_failureTolerant(image.columns(), image.rows(), m_sixteenBit ? 8 : 4);

            if (!data)
            {
                qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Failed to allocate memory for loading" << filePath;
                loadingFailed();

                return false;
            }

            image.write(0, 0, image.columns(), image.rows(), "BGRA", type, reinterpret_cast<void*>(data));

            if (observer)
            {
                observer->progressInfo(0.9F);
            }

            if (m_loadFlags & LoadICCData)
            {
                Blob iccBlob(image.iccColorProfile());
                QByteArray iccRawProfile(reinterpret_cast<const char*>(iccBlob.data()), iccBlob.length());

                if (!iccRawProfile.isEmpty())
                {
                    imageSetIccProfile(IccProfile(iccRawProfile));
                }
            }

            if (observer)
            {
                observer->progressInfo(1.0F);
            }

            imageWidth()  = image.columns();
            imageHeight() = image.rows();
            imageData()   = data;

#if MagickLibVersion < 0x700

            m_hasAlpha    = image.matte();

#else

            m_hasAlpha    = image.alpha();

#endif

            // We considering that PNG is the most representative format of an image loaded by ImageMagick

            imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
            imageSetAttribute(QLatin1String("originalColorModel"), DImg::RGB);
            imageSetAttribute(QLatin1String("originalBitDepth"),   m_sixteenBit ? 16 : 8);
            imageSetAttribute(QLatin1String("originalSize"),       QSize(image.columns(), image.rows()));
        }
        else
        {
            try
            {
                image.ping(filePath.toUtf8().constData());
            }
            catch (Warning& warning)
            {
                qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick warning [" << filePath << "]" << warning.what();
            }

            imageWidth()  = image.columns();
            imageHeight() = image.rows();

#if MagickLibVersion < 0x700

            m_hasAlpha    = image.matte();

#else

            m_hasAlpha    = image.alpha();

#endif

            m_sixteenBit  = (image.depth() == 16);

            imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
            imageSetAttribute(QLatin1String("originalColorModel"), DImg::RGB);
            imageSetAttribute(QLatin1String("originalBitDepth"),   m_sixteenBit ? 16 : 8);
            imageSetAttribute(QLatin1String("originalSize"),       QSize(image.columns(), image.rows()));
        }
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick exception [" << filePath << "]" << error.what();
        loadingFailed();

        return false;
    }

    return true;
}

} // namespace DigikamImageMagickDImgPlugin
