/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-14
 * Description : A JPEG-2000 IO file for DImg framework- load operations
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjpeg2000loader_p.h"

namespace DigikamJPEG2000DImgPlugin
{

bool DImgJPEG2000Loader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath);

#ifndef __clang_analyzer__

    // Disable false-positive memory leak reported by scan-build with file stream.

#ifdef Q_OS_WIN

    FILE* const file = _wfopen((const wchar_t*)filePath.utf16(), L"rb");

#else

    FILE* const file = fopen(filePath.toUtf8().constData(), "rb");

#endif

    if (!file)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to open JPEG2000 file";
        loadingFailed();

        return false;
    }

    imageSetAttribute(QLatin1String("format"), QLatin1String("JP2"));

#if defined JAS_VERSION_MAJOR && JAS_VERSION_MAJOR >= 3

    // NOTE: nothing to do.

#else

    QScopedPointer<DMetadata> metadata(new DMetadata(filePath));
    QSize size             = metadata->getItemDimensions();
    QString decoderOptions = QLatin1String("max_samples=100000000");

    if (size.isValid())
    {
        imageWidth()   = size.width();
        imageHeight()  = size.height();
        decoderOptions = QString::fromLatin1("max_samples=%1").arg(size.width() * size.height() * 4);
        qCDebug(DIGIKAM_DIMG_LOG_JP2K) << "JP2 image size:" << size;
    }

    if (!(m_loadFlags & LoadImageData) && !(m_loadFlags & LoadICCData))
    {
        // libjasper will load the full image in memory already when calling jas_image_decode.
        // This is bad when scanning. See bugs 215458 and 195583.
        // Exiv2 is used to extract this info.

        fclose(file);

        return true;
    }

#endif

    // -------------------------------------------------------------------
    // Initialize JPEG 2000 API.

    long           i                       = 0;
    long           x                       = 0;
    long           y                       = 0;
    int            components[4]           = { 0 };
    unsigned int   maximum_component_depth = 0;
    unsigned int   scale[4]                = { 0 };
    unsigned int   x_step[4]               = { 0 };
    unsigned int   y_step[4]               = { 0 };
    unsigned long  number_components       = 0;
    jas_image_t*  jp2_image                = nullptr;
    jas_stream_t* jp2_stream               = nullptr;
    jas_matrix_t* pixels[4]                = { nullptr };

    if (initJasper() != 0)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to init JPEG2000 decoder";
        loadingFailed();
        fclose(file);

        return false;
    }

    jp2_stream = jas_stream_freopen(filePath.toUtf8().constData(), "rb", file);

    if (jp2_stream == nullptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to open JPEG2000 stream";
        fclose(file);

        cleanupJasper();
        loadingFailed();

        return false;
    }

    // NOTE: at this point, libjasper takes owner to the file descriptor and it's no need to
    // call fclose() anymore else a double free corruption appears.

    int fmt   = jas_image_strtofmt(QByteArray("jp2").data());

#if defined JAS_VERSION_MAJOR && JAS_VERSION_MAJOR >= 3

    jp2_image = jas_image_decode(jp2_stream, fmt, nullptr);

#else

    // See bug 447240 and UPSTREAM https://github.com/jasper-software/jasper/issues/315#issuecomment-1007872809

    qCDebug(DIGIKAM_DIMG_LOG_JP2K) << "jas_image_decode decoder options string:" << decoderOptions;

    jp2_image = jas_image_decode(jp2_stream, fmt, decoderOptions.toLatin1().data());

#endif

    if (jp2_image == nullptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to decode JPEG2000 image";
        jas_stream_close(jp2_stream);

        cleanupJasper();
        loadingFailed();

        return false;
    }

    jas_stream_close(jp2_stream);

#endif // __clang_analyzer__

    // some pseudo-progress

    if (observer)
    {
        observer->progressInfo(0.1F);
    }

    // -------------------------------------------------------------------
    // Check color space.

    int colorModel = DImg::COLORMODELUNKNOWN;

    switch (jas_clrspc_fam(jas_image_clrspc(jp2_image)))
    {
        case JAS_CLRSPC_FAM_RGB:
        {
            components[0] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_RGB_R);
            components[1] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_RGB_G);
            components[2] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_RGB_B);

            if ((components[0] < 0) || (components[1] < 0) || (components[2] < 0))
            {
                qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error parsing JPEG2000 image : Missing Image Channel";
                jas_image_destroy(jp2_image);

                cleanupJasper();
                loadingFailed();

                return false;
            }

            number_components = 3;
            components[3]     = jas_image_getcmptbytype(jp2_image, 3);

            if (components[3] > 0)
            {
                m_hasAlpha = true;
                ++number_components;
            }

            colorModel = DImg::RGB;

            break;
        }

        case JAS_CLRSPC_FAM_GRAY:
        {
            components[0] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_GRAY_Y);

            // cppcheck-suppress knownConditionTrueFalse
            if (components[0] < 0)
            {
                qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error parsing JP2000 image : Missing Image Channel";
                jas_image_destroy(jp2_image);

                cleanupJasper();
                loadingFailed();

                return false;
            }

            number_components = 1;
            colorModel        = DImg::GRAYSCALE;

            break;
        }

        case JAS_CLRSPC_FAM_YCBCR:
        {
            components[0] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_YCBCR_Y);
            components[1] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_YCBCR_CB);
            components[2] = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_YCBCR_CR);

            // cppcheck-suppress knownConditionTrueFalse
            if ((components[0] < 0) || (components[1] < 0) || (components[2] < 0))
            {
                qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error parsing JP2000 image : Missing Image Channel";
                jas_image_destroy(jp2_image);

                cleanupJasper();
                loadingFailed();

                return false;
            }

            number_components = 3;
            components[3]     = jas_image_getcmptbytype(jp2_image, JAS_IMAGE_CT_UNKNOWN);

            if (components[3] > 0)
            {
                m_hasAlpha = true;
                ++number_components;
            }

            // FIXME: image->colorspace = YCbCrColorspace;

            colorModel = DImg::YCBCR;

            break;
        }

        default:
        {
            qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error parsing JP2000 image : Colorspace Model Is Not Supported";
            jas_image_destroy(jp2_image);

            cleanupJasper();
            loadingFailed();

            return false;
        }
    }

    // -------------------------------------------------------------------
    // Check image geometry.

    imageWidth()  = jas_image_width(jp2_image);
    imageHeight() = jas_image_height(jp2_image);

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        if (
            (
             ((jas_image_cmptwidth(jp2_image, components[i])*
               jas_image_cmpthstep(jp2_image, components[i])) != (long)imageWidth())
            )  ||
            (
             ((jas_image_cmptheight(jp2_image, components[i])*
               jas_image_cmptvstep(jp2_image, components[i])) != (long)imageHeight())
            ) ||
            (jas_image_cmpttlx(jp2_image, components[i])  != 0)                        ||
            (jas_image_cmpttly(jp2_image, components[i])  != 0)                        ||
            (jas_image_cmptsgnd(jp2_image, components[i]) != false)
           )
        {
            qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error parsing JPEG2000 image : Irregular Channel Geometry Not Supported";
            jas_image_destroy(jp2_image);

            cleanupJasper();
            loadingFailed();

            return false;
        }

        x_step[i] = jas_image_cmpthstep(jp2_image, components[i]);
        y_step[i] = jas_image_cmptvstep(jp2_image, components[i]);
    }

    // -------------------------------------------------------------------
    // Get image format.

    maximum_component_depth = 0;

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        maximum_component_depth = qMax((long)jas_image_cmptprec(jp2_image, components[i]),
                                       (long)maximum_component_depth);

        pixels[i]               = jas_matrix_create(1, ((unsigned int)imageWidth()) / x_step[i]);

        if (!pixels[i])
        {
            qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error decoding JPEG2000 image data : Memory Allocation Failed";
            jas_image_destroy(jp2_image);

            cleanupJasper();
            loadingFailed();

            return false;
        }
    }

#if defined JAS_VERSION_MAJOR && JAS_VERSION_MAJOR >= 3

    if (!(m_loadFlags & LoadImageData) && !(m_loadFlags & LoadICCData))
    {
        imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
        imageSetAttribute(QLatin1String("originalBitDepth"),   maximum_component_depth);
        imageSetAttribute(QLatin1String("originalSize"),       QSize(imageWidth(), imageHeight()));

        jas_image_destroy(jp2_image);
        cleanupJasper();

        return true;
    }

#endif

    if (maximum_component_depth > 8)
    {
        m_sixteenBit = true;
    }

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        scale[i] = 1;
        int prec = jas_image_cmptprec(jp2_image, components[i]);

        if (m_sixteenBit && (prec < 16))
        {
            scale[i] = (1 << (16 - jas_image_cmptprec(jp2_image, components[i])));
        }
    }

    // -------------------------------------------------------------------
    // Get image data.

    std::unique_ptr<uchar> data;

    if (m_loadFlags & LoadImageData)
    {
        if (m_sixteenBit)          // 16 bits image.
        {
            data.reset(new_failureTolerant(imageWidth(), imageHeight(), 8));
        }
        else
        {
            data.reset(new_failureTolerant(imageWidth(), imageHeight(), 4));
        }

        if (!data)
        {
            qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error decoding JPEG2000 image data : Memory Allocation Failed";
            jas_image_destroy(jp2_image);

            for (i = 0 ; i < (long)number_components ; ++i)
            {
                jas_matrix_destroy(pixels[i]);
            }

            cleanupJasper();
            loadingFailed();

            return false;
        }

        uint   checkPoint     = 0;
        uchar* dst            = data.get();
        unsigned short* dst16 = reinterpret_cast<unsigned short*>(data.get());

        for (y = 0 ; y < (long)imageHeight() ; ++y)
        {
            for (i = 0 ; i < (long)number_components ; ++i)
            {
                int ret = jas_image_readcmpt(jp2_image, (short)components[i], 0,
                                             ((unsigned int) y)            / y_step[i],
                                             ((unsigned int) imageWidth()) / x_step[i],
                                             1, pixels[i]);

                if (ret != 0)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error decoding JPEG2000 image data";
                    jas_image_destroy(jp2_image);

                    for (i = 0 ; i < (long)number_components ; ++i)
                    {
                        jas_matrix_destroy(pixels[i]);
                    }

                    cleanupJasper();
                    loadingFailed();

                    return false;
                }
            }

            switch (number_components)
            {
                case 1: // Grayscale.
                {
                    if (!m_sixteenBit)   // 8 bits image.
                    {
                        for (x = 0 ; x < (long)imageWidth() ; ++x)
                        {
                            dst[0] = (uchar)(scale[0] * jas_matrix_getv(pixels[0], x / x_step[0]));
                            dst[1] = dst[0];
                            dst[2] = dst[0];
                            dst[3] = 0xFF;

                            dst += 4;
                        }
                    }
                    else                // 16 bits image.
                    {
                        for (x = 0 ; x < (long)imageWidth() ; ++x)
                        {
                            dst16[0] = (unsigned short)(scale[0] * jas_matrix_getv(pixels[0], x / x_step[0]));
                            dst16[1] = dst16[0];
                            dst16[2] = dst16[0];
                            dst16[3] = 0xFFFF;

                            dst16 += 4;
                        }
                    }

                    break;
                }

                case 3: // RGB.
                {
                    if (!m_sixteenBit)   // 8 bits image.
                    {
                        for (x = 0 ; x < (long)imageWidth() ; ++x)
                        {
                            // Blue
                            dst[0] = (uchar)(scale[2] * jas_matrix_getv(pixels[2], x / x_step[2]));
                            // Green
                            dst[1] = (uchar)(scale[1] * jas_matrix_getv(pixels[1], x / x_step[1]));
                            // Red
                            dst[2] = (uchar)(scale[0] * jas_matrix_getv(pixels[0], x / x_step[0]));
                            // Alpha
                            dst[3] = 0xFF;

                            dst += 4;
                        }
                    }
                    else                // 16 bits image.
                    {
                        for (x = 0 ; x < (long)imageWidth() ; ++x)
                        {
                            // Blue
                            dst16[0] = (unsigned short)(scale[2] * jas_matrix_getv(pixels[2], x / x_step[2]));
                            // Green
                            dst16[1] = (unsigned short)(scale[1] * jas_matrix_getv(pixels[1], x / x_step[1]));
                            // Red
                            dst16[2] = (unsigned short)(scale[0] * jas_matrix_getv(pixels[0], x / x_step[0]));
                            // Alpha
                            dst16[3] = 0xFFFF;

                            dst16 += 4;
                        }
                    }

                    break;
                }

                case 4: // RGBA.
                {
                    if (!m_sixteenBit)   // 8 bits image.
                    {
                        for (x = 0 ; x < (long)imageWidth() ; ++x)
                        {
                            // Blue
                            dst[0] = (uchar)(scale[2] * jas_matrix_getv(pixels[2], x / x_step[2]));
                            // Green
                            dst[1] = (uchar)(scale[1] * jas_matrix_getv(pixels[1], x / x_step[1]));
                            // Red
                            dst[2] = (uchar)(scale[0] * jas_matrix_getv(pixels[0], x / x_step[0]));
                            // Alpha
                            dst[3] = (uchar)(scale[3] * jas_matrix_getv(pixels[3], x / x_step[3]));

                            dst += 4;
                        }
                    }
                    else                // 16 bits image.
                    {
                        for (x = 0 ; x < (long)imageWidth() ; ++x)
                        {
                            // Blue
                            dst16[0] = (unsigned short)(scale[2] * jas_matrix_getv(pixels[2], x / x_step[2]));
                            // Green
                            dst16[1] = (unsigned short)(scale[1] * jas_matrix_getv(pixels[1], x / x_step[1]));
                            // Red
                            dst16[2] = (unsigned short)(scale[0] * jas_matrix_getv(pixels[0], x / x_step[0]));
                            // Alpha
                            dst16[3] = (unsigned short)(scale[3] * jas_matrix_getv(pixels[3], x / x_step[3]));

                            dst16 += 4;
                        }
                    }

                    break;
                }
            }

            // Use 0-10% and 90-100% for pseudo-progress

            if (observer && (y >= (long)checkPoint))
            {
                checkPoint += granularity(observer, y, 0.8F);

                if (!observer->continueQuery())
                {
                    jas_image_destroy(jp2_image);

                    for (i = 0 ; i < (long)number_components ; ++i)
                    {
                        jas_matrix_destroy(pixels[i]);
                    }

                    cleanupJasper();
                    loadingFailed();

                    return false;
                }

                observer->progressInfo(0.1F + (0.8F * (((float)y) / ((float)imageHeight()))));
            }
        }
    }

    // -------------------------------------------------------------------
    // Get ICC color profile.

    if (m_loadFlags & LoadICCData)
    {
        jas_iccprof_t* icc_profile = nullptr;
        jas_stream_t*  icc_stream  = nullptr;
        jas_cmprof_t*  cm_profile  = nullptr;

        // To prevent cppcheck warnings.

        (void)icc_profile;
        (void)icc_stream;
        (void)cm_profile;

        cm_profile = jas_image_cmprof(jp2_image);

        if (cm_profile != nullptr)
        {
            icc_profile = jas_iccprof_createfromcmprof(cm_profile);
        }

        if (icc_profile != nullptr)
        {
            icc_stream = jas_stream_memopen(nullptr, 0);

            if (icc_stream != nullptr)
            {
                if (jas_iccprof_save(icc_profile, icc_stream) == 0)
                {
                    if (jas_stream_flush(icc_stream) == 0)
                    {
                        jas_stream_memobj_t* const blob = reinterpret_cast<jas_stream_memobj_t*>(icc_stream->obj_);
                        QByteArray profile_rawdata;
                        profile_rawdata.resize(blob->len_);
                        memcpy(profile_rawdata.data(), blob->buf_, blob->len_);
                        imageSetIccProfile(IccProfile(profile_rawdata));
                        jas_stream_close(icc_stream);
                    }
                }
            }
        }
        else
        {
            // If ICC profile is null, check Exif metadata.

            checkExifWorkingColorSpace();
        }
    }

    if (observer)
    {
        observer->progressInfo(1.0F);
    }

    imageData() = data.release();
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   maximum_component_depth);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(imageWidth(), imageHeight()));

    jas_image_destroy(jp2_image);

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        jas_matrix_destroy(pixels[i]);
    }

    cleanupJasper();

    return true;
}

} // namespace DigikamJPEG2000DImgPlugin
