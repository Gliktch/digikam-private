/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-17
 * Description : A TIFF IO file for DImg framework - load operations
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
#include <cmath>

// Qt includes

#include <QFile>
#include <QFloat16>
#include <QByteArray>

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace DigikamTIFFDImgPlugin
{

bool DImgTIFFLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath);

    // -------------------------------------------------------------------
    // TIFF error handling. If an errors/warnings occurs during reading,
    // libtiff will call these methods

#ifdef Q_OS_WIN

    TIFFSetWarningHandler(NULL);        // krazy:exclude=null

#else

    TIFFSetWarningHandler(dimg_tiff_warning);

#endif

    TIFFSetErrorHandler(dimg_tiff_error);

    // -------------------------------------------------------------------
    // Open the file

#ifdef Q_OS_WIN

    TIFF* const tif = TIFFOpenW((const wchar_t*)filePath.utf16(), "r");

#else

    TIFF* const tif = TIFFOpen(filePath.toUtf8().constData(), "r");

#endif

    if (!tif)
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Cannot open image file.";
        loadingFailed();

        return false;
    }
/*
    if (DIGIKAM_DIMG_LOG_TIFF().isDebugEnabled())
    {
        TIFFPrintDirectory(tif, stdout, 0);
    }
*/
    // -------------------------------------------------------------------
    // Get image information.

    uint32    w, h;
    uint16    bits_per_sample;
    uint16    samples_per_pixel;
    uint16    sample_format;
    uint16    photometric;
    uint16    planar_config;
    uint32    rows_per_strip;
    tsize_t   strip_size;
    tstrip_t  num_of_strips;

    TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGELENGTH, &h);

    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sample_format);
    TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planar_config);

    if ((TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip) == 0) || (rows_per_strip == 0))
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "TIFF loader: Cannot handle non-stripped images. Loading file "
                                         << filePath;
        TIFFClose(tif);
        loadingFailed();

        return false;
    }

    if (rows_per_strip > h)
    {
        rows_per_strip = h;
    }

    if (
        (bits_per_sample   == 0) ||
        (samples_per_pixel == 0) ||
        (rows_per_strip    == 0)
/*
        || (rows_per_strip >  h)
*/
       )
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "TIFF loader: Encountered invalid value in image." << Qt::endl
                                         << " bits_per_sample   : " << bits_per_sample         << Qt::endl
                                         << " samples_per_pixel : " << samples_per_pixel       << Qt::endl
                                         << " rows_per_strip    : " << rows_per_strip          << Qt::endl
                                         << " h                 : " << h                       << Qt::endl
                                         << " Loading file      : " << filePath;
        TIFFClose(tif);
        loadingFailed();

        return false;
    }

    // TODO: check others TIFF color-spaces here. Actually, only RGB, PALETTE and MINISBLACK
    // have been tested.
    // Complete description of TIFFTAG_PHOTOMETRIC tag can be found at this Url:
    // www.awaresystems.be/imaging/tiff/tifftags/photometricinterpretation.html

    TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &photometric);

    if (
         (photometric != PHOTOMETRIC_RGB)                                 &&
         (photometric != PHOTOMETRIC_PALETTE)                             &&
         (photometric != PHOTOMETRIC_MINISWHITE)                          &&
         (photometric != PHOTOMETRIC_MINISBLACK)                          &&
         (
          (photometric != PHOTOMETRIC_YCBCR) || (bits_per_sample != 8)
         ) &&
         (
           (photometric != PHOTOMETRIC_SEPARATED) || (bits_per_sample != 8)
         ) &&
         (m_loadFlags & LoadImageData)
       )
    {
        qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Can not handle image without RGB color-space: "
                                         << photometric;
        TIFFClose(tif);
        loadingFailed();

        return false;
    }

    int colorModel = DImg::COLORMODELUNKNOWN;

    switch (photometric)
    {
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
        {
            colorModel = DImg::GRAYSCALE;
            break;
        }

        case PHOTOMETRIC_RGB:
        {
            colorModel = DImg::RGB;
            break;
        }

        case PHOTOMETRIC_PALETTE:
        {
            colorModel = DImg::INDEXED;
            break;
        }

        case PHOTOMETRIC_MASK:
        {
            colorModel = DImg::MONOCHROME;
            break;
        }

        case PHOTOMETRIC_SEPARATED:
        {
            colorModel = DImg::CMYK;
            break;
        }

        case PHOTOMETRIC_YCBCR:
        {
            colorModel = DImg::YCBCR;
            break;
        }

        case PHOTOMETRIC_CIELAB:
        case PHOTOMETRIC_ICCLAB:
        case PHOTOMETRIC_ITULAB:
        {
            colorModel = DImg::CIELAB;
            break;
        }

        case PHOTOMETRIC_LOGL:
        case PHOTOMETRIC_LOGLUV:
        {
            colorModel = DImg::COLORMODELRAW;
            break;
        }
    }

    if (samples_per_pixel == 4)
    {
        m_hasAlpha = true;
    }
    else
    {
        m_hasAlpha = false;
    }

    if ((bits_per_sample == 16) || (bits_per_sample == 32))
    {
        m_sixteenBit = true;
    }
    else
    {
        m_sixteenBit = false;
    }

    // -------------------------------------------------------------------
    // Read image ICC profile

    if (m_loadFlags & LoadICCData)
    {
        uchar*  profile_data = nullptr;
        uint32  profile_size;

        if (TIFFGetField(tif, TIFFTAG_ICCPROFILE, &profile_size, &profile_data))
        {
            QByteArray profile_rawdata;
            profile_rawdata.resize(profile_size);
            memcpy(profile_rawdata.data(), profile_data, profile_size);
            imageSetIccProfile(IccProfile(profile_rawdata));
        }
        else
        {
            // If ICC profile is null, check Exif metadata.

            checkExifWorkingColorSpace();
        }
    }

    // -------------------------------------------------------------------
    // Get image data.

    QScopedArrayPointer<uchar> data;

    if (m_loadFlags & LoadImageData)
    {
        if (observer)
        {
            observer->progressInfo(0.1F);
        }

        strip_size    = TIFFStripSize(tif);
        num_of_strips = TIFFNumberOfStrips(tif);

        if (bits_per_sample == 16)          // 16 bits image.
        {
            data.reset(new_failureTolerant(w, h, 8));
            QScopedArrayPointer<uchar> strip(new_failureTolerant(strip_size));

            if (!data || strip.isNull())
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to allocate memory for TIFF image" << filePath;
                TIFFClose(tif);
                loadingFailed();

                return false;
            }

            qint64 offset     = 0;
            qint64 bytesRead  = 0;
            uint   checkpoint = 0;

            for (tstrip_t st = 0 ; st < num_of_strips ; ++st)
            {
                if (observer && (st == checkpoint))
                {
                    checkpoint += granularity(observer, num_of_strips, 0.8F);

                    if (!observer->continueQuery())
                    {
                        TIFFClose(tif);
                        loadingFailed();

                        return false;
                    }

                    observer->progressInfo(0.1F + (0.8F * (((float)st) / ((float)num_of_strips))));
                }

                bytesRead = TIFFReadEncodedStrip(tif, st, strip.data(), strip_size);

                if (bytesRead == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read strip";
                    break;
                }

                if ((num_of_strips != 0) && (samples_per_pixel != 0))
                {
                    if ((planar_config == PLANARCONFIG_SEPARATE) &&
                        (remainder((double)st, (double)(num_of_strips / samples_per_pixel)) == 0.0))
                    {
                        offset = 0;
                    }
                }

                ushort* stripPtr = reinterpret_cast<ushort*>(strip.data());         // cppcheck-suppress constVariablePointer
                ushort* dataPtr  = reinterpret_cast<ushort*>(data.data() + offset);
                ushort* p        = nullptr;

                // tiff data is read as BGR or ABGR or Greyscale

                if      (samples_per_pixel == 1)   // See bug #148400: Greyscale pictures only have _one_ sample per pixel
                {
                    for (int i = 0 ; i < (bytesRead / 2) ; ++i)
                    {
                        // We have to read two bytes for one pixel

                        p = dataPtr;

                        if (sample_format == SAMPLEFORMAT_IEEEFP)
                        {
                            p[0] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr))   * 65535.0F, 65535.0F);
                            p[1] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr))   * 65535.0F, 65535.0F);
                            p[2] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[3] = 0xFFFF;
                        }
                        else
                        {
                            p[0] = *stripPtr;     // RGB have to be set to the _same_ value
                            p[1] = *stripPtr;
                            p[2] = *stripPtr++;
                            p[3] = 0xFFFF;        // set alpha to 100%
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead * 4;      // The _byte_offset in the data array is, of course, four times bytesRead
                }

                else if ((samples_per_pixel == 3) &&
                         (planar_config == PLANARCONFIG_CONTIG))
                {
                    for (int i = 0 ; i < (bytesRead / 6) ; ++i)
                    {
                        p = dataPtr;

                        if (sample_format == SAMPLEFORMAT_IEEEFP)
                        {
                            p[2] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[1] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[0] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[3] = 0xFFFF;
                        }
                        else
                        {
                            p[2] = *stripPtr++;
                            p[1] = *stripPtr++;
                            p[0] = *stripPtr++;
                            p[3] = 0xFFFF;
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead / 6 * 8;
                }

                // cppcheck-suppress knownConditionTrueFalse
                else if ((samples_per_pixel == 3) &&
                         (planar_config == PLANARCONFIG_SEPARATE))
                {
                    for (int i = 0 ; i < (bytesRead / 2) ; ++i)
                    {
                        p = dataPtr;

                        // cppcheck-suppress knownConditionTrueFalse
                        if (samples_per_pixel != 0)
                        {
                            int den = (int)num_of_strips / (int)samples_per_pixel;

                            if (den != 0)
                            {
                                int val = st / den;

                                if (sample_format == SAMPLEFORMAT_IEEEFP)
                                {
                                    switch (val)
                                    {
                                        case 0:
                                        {
                                            p[2] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            p[3] = 0xFFFF;
                                            break;
                                        }

                                        case 1:
                                        {
                                            p[1] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            break;
                                        }

                                        case 2:
                                        {
                                            p[0] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    switch (val)
                                    {
                                        case 0:
                                        {
                                            p[2] = *stripPtr++;
                                            p[3] = 0xFFFF;
                                            break;
                                        }

                                        case 1:
                                        {
                                            p[1] = *stripPtr++;
                                            break;
                                        }

                                        case 2:
                                        {
                                            p[0] = *stripPtr++;
                                            break;
                                        }
                                    }
                                }

                                dataPtr += 4;
                            }
                        }
                    }

                    offset += bytesRead / 2 * 8;
                }

                else if ((samples_per_pixel == 4) &&
                         (planar_config == PLANARCONFIG_CONTIG))
                {
                    for (int i = 0 ; i < (bytesRead / 8) ; ++i)
                    {
                        p = dataPtr;

                        if (sample_format == SAMPLEFORMAT_IEEEFP)
                        {
                            p[2] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[1] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[0] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                            p[3] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                        }
                        else
                        {
                            p[2] = *stripPtr++;
                            p[1] = *stripPtr++;
                            p[0] = *stripPtr++;
                            p[3] = *stripPtr++;
                        }

                        dataPtr += 4;
                    }

                    offset += bytesRead;
                }

                // cppcheck-suppress knownConditionTrueFalse
                else if ((samples_per_pixel == 4) &&
                         (planar_config == PLANARCONFIG_SEPARATE))
                {
                    for (int i = 0 ; i < (bytesRead / 2) ; ++i)
                    {
                        p = dataPtr;

                        // cppcheck-suppress knownConditionTrueFalse
                        if (samples_per_pixel != 0)
                        {
                            int den = (int)num_of_strips / (int)samples_per_pixel;

                            if (den != 0)
                            {
                                int val = st / den;

                                if (sample_format == SAMPLEFORMAT_IEEEFP)
                                {
                                    switch (val)
                                    {
                                        case 0:
                                        {
                                            p[2] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            break;
                                        }

                                        case 1:
                                        {
                                            p[1] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            break;
                                        }

                                        case 2:
                                        {
                                            p[0] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            break;
                                        }

                                        case 3:
                                        {
                                            p[3] = (ushort)qBound(0.0F, (*reinterpret_cast<qfloat16*>(stripPtr++)) * 65535.0F, 65535.0F);
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    switch (val)
                                    {
                                        case 0:
                                        {
                                            p[2] = *stripPtr++;
                                            break;
                                        }

                                        case 1:
                                        {
                                            p[1] = *stripPtr++;
                                            break;
                                        }

                                        case 2:
                                        {
                                            p[0] = *stripPtr++;
                                            break;
                                        }

                                        case 3:
                                        {
                                            p[3] = *stripPtr++;
                                            break;
                                        }
                                    }
                                }

                                dataPtr += 4;
                            }
                        }
                    }

                    offset += bytesRead / 2 * 8;
                }
            }
        }

        else if ((bits_per_sample == 32) && (sample_format == SAMPLEFORMAT_IEEEFP))          // 32 bits float image.
        {
            data.reset(new_failureTolerant(w, h, 8));
            QScopedArrayPointer<uchar> strip(new_failureTolerant(strip_size));

            if (!data || strip.isNull())
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to allocate memory for TIFF image" << filePath;
                TIFFClose(tif);
                loadingFailed();

                return false;
            }

            qint64 offset     = 0;
            qint64 bytesRead  = 0;
            uint   checkpoint = 0;
            float  maxValue   = 0.0;

            for (tstrip_t st = 0 ; st < num_of_strips ; ++st)
            {
                if (observer && !observer->continueQuery())
                {
                    TIFFClose(tif);
                    loadingFailed();

                    return false;
                }

                bytesRead = TIFFReadEncodedStrip(tif, st, strip.data(), strip_size);

                if (bytesRead == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read strip";
                    break;
                }

                const float* stripPtr = reinterpret_cast<float*>(strip.data());

                for (int i = 0 ; i < (bytesRead / 4) ; ++i)
                {
                    maxValue = qMax(maxValue, *stripPtr++);
                }
            }

            double factor = (maxValue > 10.0) ? log10(maxValue) * 1.5 : 1.0;    // cppcheck-suppress invalidFunctionArg
            double scale  = (factor > 1.0)    ? 0.75                  : 1.0;

            if (factor > 1.0)
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "TIFF image cannot be converted lossless from 32 to 16 bits" << filePath;
            }

            for (tstrip_t st = 0 ; st < num_of_strips ; ++st)
            {
                if (observer && st == checkpoint)
                {
                    checkpoint += granularity(observer, num_of_strips, 0.8F);

                    if (!observer->continueQuery())
                    {
                        TIFFClose(tif);
                        loadingFailed();

                        return false;
                    }

                    observer->progressInfo(0.1F + (0.8F * (((float)st) / ((float)num_of_strips))));
                }

                bytesRead = TIFFReadEncodedStrip(tif, st, strip.data(), strip_size);

                if (bytesRead == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read strip";
                    break;
                }


                if ((num_of_strips != 0) && (samples_per_pixel != 0))
                {
                    if (
                        (planar_config == PLANARCONFIG_SEPARATE) &&
                        (remainder((double)st, (double)(num_of_strips / samples_per_pixel)) == 0.0)
                       )
                    {
                        offset = 0;
                    }
                }

                const float*  stripPtr = reinterpret_cast<float*>(strip.data());
                ushort*       dataPtr  = reinterpret_cast<ushort*>(data.data() + offset);
                ushort*       p        = nullptr;

                if      (
                         (samples_per_pixel == 3) &&
                         (planar_config == PLANARCONFIG_CONTIG)
                        )
                {
                    for (int i = 0 ; i < (bytesRead / 12) ; ++i)
                    {
                        p = dataPtr;

                        p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[3] = 0xFFFF;

                        dataPtr += 4;
                    }

                    offset += bytesRead / 12 * 8;
                }

                // cppcheck-suppress knownConditionTrueFalse
                else if (
                         (samples_per_pixel == 3) &&
                         (planar_config == PLANARCONFIG_SEPARATE)
                        )
                {
                    for (int i = 0 ; i < (bytesRead / 4) ; ++i)
                    {
                        p = dataPtr;

                        // cppcheck-suppress knownConditionTrueFalse
                        if (samples_per_pixel != 0)
                        {
                            int den = (int)num_of_strips / (int)samples_per_pixel;

                            if (den != 0)
                            {
                                int val = st / den;

                                switch (val)
                                {
                                    case 0:
                                    {
                                        p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                        p[3] = 0xFFFF;
                                        break;
                                    }

                                    case 1:
                                    {
                                        p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                        break;
                                    }

                                    case 2:
                                    {
                                        p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                        break;
                                    }
                                }

                                dataPtr += 4;
                            }
                        }
                    }

                    offset += bytesRead / 4 * 8;
                }

                else if (
                         (samples_per_pixel == 4) &&
                         (planar_config == PLANARCONFIG_CONTIG)
                        )
                {
                    for (int i = 0 ; i < (bytesRead / 16) ; ++i)
                    {
                        p = dataPtr;

                        p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                        p[3] = (ushort)qBound(0.0, (double)*stripPtr++ * 65535.0, 65535.0);

                        dataPtr += 4;
                    }

                    offset += bytesRead / 16 * 8;
                }

                // cppcheck-suppress knownConditionTrueFalse
                else if (
                         (samples_per_pixel == 4) &&
                         (planar_config == PLANARCONFIG_SEPARATE)
                        )
                {
                    for (int i = 0 ; i < bytesRead / 4 ; ++i)
                    {
                        p = dataPtr;

                        // cppcheck-suppress knownConditionTrueFalse
                        if (samples_per_pixel != 0)
                        {
                            int den = (int)num_of_strips / (int)samples_per_pixel;

                            if (den != 0)
                            {
                                int val = st / den;

                                switch (val)
                                {
                                    case 0:
                                    {
                                        p[2] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                        break;
                                    }

                                    case 1:
                                    {
                                        p[1] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                        break;
                                    }

                                    case 2:
                                    {
                                        p[0] = (ushort)qBound(0.0, pow((double)*stripPtr++ / factor, scale) * 65535.0, 65535.0);
                                        break;
                                    }

                                    case 3:
                                    {
                                        p[3] = (ushort)qBound(0.0, (double)*stripPtr++ * 65535.0, 65535.0);
                                        break;
                                    }
                                }

                                dataPtr += 4;
                            }
                        }
                    }

                    offset += bytesRead / 4 * 8;
                }
            }
        }

        else       // Non 16 or 32 bits images ==> get it on BGRA 8 bits.
        {
            data.reset(new_failureTolerant(w, h, 4));
            QScopedArrayPointer<uchar> strip(new_failureTolerant(w, rows_per_strip, 4));

            if (!data || strip.isNull())
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to allocate memory for TIFF image" << filePath;
                TIFFClose(tif);
                loadingFailed();

                return false;
            }

            qint64 offset       = 0;
            qint64 pixelsRead   = 0;

            // this is inspired by TIFFReadRGBAStrip, tif_getimage.c

            char emsg[1024]     = "";
            uint32 rows_to_read = 0;
            uint checkpoint     = 0;
            TIFFRGBAImage img;

            // test whether libtiff can read format and initiate reading

            if (!TIFFRGBAImageOK(tif, emsg) || !TIFFRGBAImageBegin(&img, tif, 0, emsg))
            {
                qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to set up RGBA reading of image, filename "
                                                 << TIFFFileName(tif) <<  " error message from Libtiff: " << emsg;
                TIFFClose(tif);
                loadingFailed();

                return false;
            }

            // libtiff cannot handle all possible orientations, it give weird results.
            // We rotate ourselves. (Bug 274865)

            img.req_orientation = img.orientation;

            // read strips from image: read rows_per_strip, so always start at beginning of a strip

            for (uint row = 0 ; row < h ; row += rows_per_strip)
            {
                if (observer && (row >= checkpoint))
                {
                    checkpoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery())
                    {
                        TIFFClose(tif);
                        loadingFailed();

                        return false;
                    }

                    observer->progressInfo(0.1F + (0.8F * (((float)row) / ((float)h))));
                }

                img.row_offset  = row;
                img.col_offset  = 0;

                if ((row + rows_per_strip) > img.height)
                {
                    rows_to_read = img.height - row;
                }
                else
                {
                    rows_to_read = rows_per_strip;
                }

                // Read data

                if (TIFFRGBAImageGet(&img, reinterpret_cast<uint32*>(strip.data()), img.width, rows_to_read) == -1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_TIFF) << "Failed to read image data";
                    break;
                }

                pixelsRead      = (qint64)rows_to_read * (qint64)img.width;

                const uchar* stripPtr = reinterpret_cast<unsigned char*>(strip.data());
                uchar*       dataPtr  = reinterpret_cast<unsigned char*>(data.data() + offset);
                uchar*       p        = nullptr;

                // Reverse red and blue

                for (int i = 0 ; i < pixelsRead ; ++i)
                {
                    p        = dataPtr;

                    p[2]     = *stripPtr++;
                    p[1]     = *stripPtr++;
                    p[0]     = *stripPtr++;
                    p[3]     = *stripPtr++;

                    dataPtr += 4;
                }

                offset += pixelsRead * 4;
            }

            TIFFRGBAImageEnd(&img);
        }
    }

    // -------------------------------------------------------------------

    TIFFClose(tif);

    if (observer)
    {
        observer->progressInfo(1.0F);
    }

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = data.take();
    imageSetAttribute(QLatin1String("format"),             QLatin1String("TIFF"));
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   bits_per_sample);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(w, h));

    return true;
}

} // namespace DigikamTIFFDImgPlugin
