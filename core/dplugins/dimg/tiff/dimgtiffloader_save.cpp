/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-17
 * Description : A TIFF IO file for DImg framework - save operations
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgtiffloader.h"

// C ANSI includes

extern "C"
{
#include <tiffvers.h>
}

// C++ includes

#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>
#include <QScopedPointer>

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace DigikamTIFFDImgPlugin
{

bool DImgTIFFLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    uint32 w    = imageWidth();
    uint32 h    = imageHeight();
    uchar* data = imageData();

    // -------------------------------------------------------------------
    // TIFF error handling. If an errors/warnings occurs during reading,
    // libtiff will call these methods

    TIFFSetWarningHandler(dimg_tiff_warning);
    TIFFSetErrorHandler(dimg_tiff_error);

    // -------------------------------------------------------------------
    // Open the file

#ifdef Q_OS_WIN

    TIFF* const tif = TIFFOpenW((const wchar_t*)filePath.utf16(), "w");

#else

    TIFF* const tif = TIFFOpen(filePath.toUtf8().constData(), "w");

#endif

    if (!tif)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot open target image file.";
        return false;
    }

    // -------------------------------------------------------------------
    // Set image properties

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,     w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH,    h);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,    PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,   PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION,    ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);

    // Image must be compressed using deflate algorithm ?

    QVariant compressAttr = imageGetAttribute(QLatin1String("compress"));
    bool compress         = compressAttr.isValid() ? compressAttr.toBool() : false;

    qCDebug(DIGIKAM_DIMG_LOG_TIFF) << "TIFF Compression Enabled:" << compress;

    if (compress)
    {
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_ADOBE_DEFLATE);
        TIFFSetField(tif, TIFFTAG_ZIPQUALITY,  9);
        // NOTE : this tag values aren't defined in libtiff 3.6.1. '2' is PREDICTOR_HORIZONTAL.
        //        Use horizontal differencing for images which are
        //        likely to be continuous tone. The TIFF spec says that this
        //        usually leads to better compression.
        //        See this Url for more details:
        //        www.awaresystems.be/imaging/tiff/tifftags/predictor.html
        TIFFSetField(tif, TIFFTAG_PREDICTOR,   2);
    }
    else
    {
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    }

    uint16 sampleinfo[1];

    if (imageHasAlpha())
    {
        sampleinfo[0] = EXTRASAMPLE_ASSOCALPHA;
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES,    1, sampleinfo);
    }
    else
    {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    }

    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)imageBitsDepth());
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,  TIFFDefaultStripSize(tif, 0));

    // -------------------------------------------------------------------
    // Write meta-data Tags contents.

    QScopedPointer<DMetadata> metaData(new DMetadata(m_image->getMetadata()));

    // Standard IPTC tag (available with libtiff 3.6.1)

    QByteArray ba = metaData->getIptc(true);

    if (!ba.isEmpty())
    {

#if defined(TIFFTAG_PHOTOSHOP)

        TIFFSetField(tif, TIFFTAG_PHOTOSHOP, (uint32)ba.size(), (uchar*)ba.data());

#endif

    }

    // Standard XMP tag (available with libtiff 3.6.1)

    if (metaData->hasXmp())
    {

#if defined(TIFFTAG_XMLPACKET)

        tiffSetExifDataTag(tif, TIFFTAG_XMLPACKET,            *metaData, "Exif.Image.XMLPacket");

#endif

    }

    // Standard Exif ASCII tags (available with libtiff 3.6.1)

    tiffSetExifAsciiTag(tif, TIFFTAG_DOCUMENTNAME,            *metaData, "Exif.Image.DocumentName");
    tiffSetExifAsciiTag(tif, TIFFTAG_IMAGEDESCRIPTION,        *metaData, "Exif.Image.ImageDescription");
    tiffSetExifAsciiTag(tif, TIFFTAG_MAKE,                    *metaData, "Exif.Image.Make");
    tiffSetExifAsciiTag(tif, TIFFTAG_MODEL,                   *metaData, "Exif.Image.Model");
    tiffSetExifAsciiTag(tif, TIFFTAG_DATETIME,                *metaData, "Exif.Image.DateTime");
    tiffSetExifAsciiTag(tif, TIFFTAG_ARTIST,                  *metaData, "Exif.Image.Artist");
    tiffSetExifAsciiTag(tif, TIFFTAG_COPYRIGHT,               *metaData, "Exif.Image.Copyright");

    QString soft       = metaData->getExifTagString("Exif.Image.Software");
    QString libtiffver = QLatin1String(TIFFLIB_VERSION_STR);
    libtiffver.replace(QLatin1Char('\n'), QLatin1Char(' '));
    soft.append(QString::fromLatin1(" ( %1 )").arg(libtiffver));
    TIFFSetField(tif, TIFFTAG_SOFTWARE, const_cast<const char*>(soft.toLatin1().constData()));

    // NOTE: All others Exif tags will be written by Exiv2 (<= 0.18)

    // -------------------------------------------------------------------
    // Write ICC profile.

    QByteArray profile_rawdata = m_image->getIccProfile().data();

    if (!profile_rawdata.isEmpty())
    {

#if defined(TIFFTAG_ICCPROFILE)

        purgeExifWorkingColorSpace();
        TIFFSetField(tif, TIFFTAG_ICCPROFILE, (uint32)profile_rawdata.size(),
                     reinterpret_cast<unsigned char*>(profile_rawdata.data()));

#endif

    }

    // -------------------------------------------------------------------
    // Write full image data in tiff directory IFD0

    if (observer)
    {
        observer->progressInfo(0.1F);
    }

    uchar*  pixel         = nullptr;
    const uint16* pixel16 = nullptr;
    double  alpha_factor  = 0;
    uint32  x             = 0;
    uint32  y             = 0;
    uint8   r8            = 0;
    uint8   g8            = 0;
    uint8   b8            = 0;
    uint8   a8            = 0;
    uint16  r16           = 0;
    uint16  g16           = 0;
    uint16  b16           = 0;
    uint16  a16           = 0;
    int     i             = 0;

    uint8* buf            = reinterpret_cast<uint8*>(_TIFFmalloc(TIFFScanlineSize(tif)));
    uint16* buf16         = nullptr;

    if (!buf)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot allocate memory buffer for main image.";
        TIFFClose(tif);

        return false;
    }

    uint checkpoint = 0;

    for (y = 0 ; y < h ; ++y)
    {

        if (observer && (y == checkpoint))
        {
            checkpoint += granularity(observer, h, 0.8F);

            if (!observer->continueQuery())
            {
                _TIFFfree(buf);
                TIFFClose(tif);

                return false;
            }

            observer->progressInfo(0.1F + (0.8F * (((float)y) / ((float)h))));
        }

        i = 0;

        for (x = 0 ; x < w ; ++x)
        {
            pixel = &data[((y * w) + x) * imageBytesDepth()];

            if (imageSixteenBit())          // 16 bits image.
            {
                pixel16 = reinterpret_cast<ushort*>(pixel);
                b16 = pixel16[0];
                g16 = pixel16[1];
                r16 = pixel16[2];

                if (imageHasAlpha())
                {
                    // TIFF makes you pre-multiply the RGB components by alpha

                    a16          = pixel16[3];
                    alpha_factor = ((double)a16 / 65535.0);
                    r16          = (uint16)(r16 * alpha_factor);
                    g16          = (uint16)(g16 * alpha_factor);
                    b16          = (uint16)(b16 * alpha_factor);
                }

                // This might be endian dependent

                buf16    = reinterpret_cast<ushort*>(buf+i);
                *buf16++ = r16;
                *buf16++ = g16;
                *buf16++ = b16;
                i       += 6;

                if (imageHasAlpha())
                {
                    *buf16++ = a16;
                    i       += 2;
                }
            }
            else                            // 8 bits image.
            {
                b8 = (uint8)pixel[0];
                g8 = (uint8)pixel[1];
                r8 = (uint8)pixel[2];

                if (imageHasAlpha())
                {
                    // TIFF makes you pre-multiply the RGB components by alpha

                    a8           = (uint8)(pixel[3]);
                    alpha_factor = ((double)a8 / 255.0);
                    r8           = (uint8)(r8 * alpha_factor);
                    g8           = (uint8)(g8 * alpha_factor);
                    b8           = (uint8)(b8 * alpha_factor);
                }

                // This might be endian dependent

                buf[i++] = r8;
                buf[i++] = g8;
                buf[i++] = b8;

                if (imageHasAlpha())
                {
                    buf[i++] = a8;
                }
            }
        }

        if (!TIFFWriteScanline(tif, buf, y, 0))
        {
            qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot write main image to target file.";
            _TIFFfree(buf);
            TIFFClose(tif);

            return false;
        }
    }

    _TIFFfree(buf);
    TIFFWriteDirectory(tif);

    // -------------------------------------------------------------------
    // Write thumbnail in tiff directory IFD1

    QImage thumb = m_image->smoothScale(160, 120, Qt::KeepAspectRatio).copyQImage();

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,      (uint32)thumb.width());
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH,     (uint32)thumb.height());
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION,     ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT,  RESUNIT_NONE);
    TIFFSetField(tif, TIFFTAG_COMPRESSION,     COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,    TIFFDefaultStripSize(tif, 0));

    uchar* pixelThumb = nullptr;
    uchar* dataThumb  = thumb.bits();
    uint8* bufThumb   = reinterpret_cast<uint8*>(_TIFFmalloc(TIFFScanlineSize(tif)));

    if (!bufThumb)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot allocate memory buffer for thumbnail.";
        TIFFClose(tif);
        return false;
    }

    for (y = 0 ; y < uint32(thumb.height()) ; ++y)
    {
        i = 0;

        for (x = 0 ; x < uint32(thumb.width()) ; ++x)
        {
            pixelThumb    = &dataThumb[((y * thumb.width()) + x) * 4];

            // This might be endian dependent

            bufThumb[i++] = (uint8)pixelThumb[2];
            bufThumb[i++] = (uint8)pixelThumb[1];
            bufThumb[i++] = (uint8)pixelThumb[0];
        }

        if (!TIFFWriteScanline(tif, bufThumb, y, 0))
        {
            qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot write thumbnail to target file.";
            _TIFFfree(bufThumb);
            TIFFClose(tif);

            return false;
        }
    }

    _TIFFfree(bufThumb);
    TIFFClose(tif);

    // -------------------------------------------------------------------

    if (observer)
    {
        observer->progressInfo(1.0F);
    }

    imageSetAttribute(QLatin1String("savedFormat"), QLatin1String("TIFF"));

    // Save metadata

    QScopedPointer<DMetadata> metaDataToFile(new DMetadata(filePath));
    metaDataToFile->setData(m_image->getMetadata());

    // See bug #211758 for these special steps needed when writing a TIFF

    metaDataToFile->removeExifThumbnail();
    metaDataToFile->removeExifTag("Exif.Image.ProcessingSoftware");
    metaDataToFile->applyChanges(true);

    return true;
}

void DImgTIFFLoader::tiffSetExifAsciiTag(TIFF* const tif,
                                         ttag_t tiffTag,
                                         const DMetadata& metaData,
                                         const char* const exifTagName)
{
    QByteArray tag = metaData.getExifTagData(exifTagName);

    if (!tag.isEmpty())
    {
        QByteArray str(tag.data(), tag.size());
        TIFFSetField(tif, tiffTag, reinterpret_cast<const char*>(str.constData()));
    }
}

void DImgTIFFLoader::tiffSetExifDataTag(TIFF* const tif,
                                        ttag_t tiffTag,
                                        const DMetadata& metaData,
                                        const char* const exifTagName)
{
    QByteArray tag = metaData.getExifTagData(exifTagName);

    if (!tag.isEmpty())
    {
        TIFFSetField(tif, tiffTag, (uint32)tag.size(), reinterpret_cast<char*>(tag.data()));
    }
}

} // namespace DigikamTIFFDImgPlugin
