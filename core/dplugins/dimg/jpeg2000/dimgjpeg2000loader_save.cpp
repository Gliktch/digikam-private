/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-14
 * Description : A JPEG-2000 IO file for DImg framework - save operations
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjpeg2000loader_p.h"

namespace DigikamJPEG2000DImgPlugin
{

bool DImgJPEG2000Loader::save(const QString& filePath, DImgLoaderObserver* const observer)
{

#ifndef __clang_analyzer__

    // Disable false-positive memory leak reported by scan-build with file stream.

#ifdef Q_OS_WIN

    FILE* const file = _wfopen((const wchar_t*)filePath.utf16(), L"wb");

#else

    FILE* const file = fopen(filePath.toUtf8().constData(), "wb");

#endif

    if (!file)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to open JPEG2000 file";

        return false;
    }

    // -------------------------------------------------------------------
    // Initialize JPEG 2000 API.

    long                 i                 = 0;
    long                 x                 = 0;
    long                 y                 = 0;
    unsigned long        number_components = 0;
    jas_image_t*         jp2_image         = nullptr;
    jas_stream_t*        jp2_stream        = nullptr;
    jas_matrix_t*        pixels[4]         = { nullptr };
    jas_image_cmptparm_t component_info[4];

    if (initJasper() != 0)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to init JPEG2000 decoder";
        fclose(file);

        return false;
    }

    jp2_stream = jas_stream_freopen(filePath.toUtf8().constData(), "wb", file);

    if (jp2_stream == nullptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to open JPEG2000 stream";
        fclose(file);

        cleanupJasper();

        return false;
    }

    // NOTE: at this point, libjasper takes owner to the file descriptor and it's no need to
    // call fclose() anymore else a double free corruption appears.

    number_components = imageHasAlpha() ? 4 : 3;

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        component_info[i].tlx    = 0;
        component_info[i].tly    = 0;
        component_info[i].hstep  = 1;
        component_info[i].vstep  = 1;
        component_info[i].width  = imageWidth();
        component_info[i].height = imageHeight();
        component_info[i].prec   = imageBitsDepth();
        component_info[i].sgnd   = false;
    }

#endif // __clang_analyzer__

    jp2_image = jas_image_create(number_components, component_info, JAS_CLRSPC_UNKNOWN);

    if (jp2_image == nullptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to create JPEG2000 image";
        jas_stream_close(jp2_stream);

        cleanupJasper();

        return false;
    }

    if (observer)
    {
        observer->progressInfo(0.1F);
    }

    // -------------------------------------------------------------------
    // Check color space.

    if (number_components >= 3)     // RGB & RGBA
    {
        // Alpha Channel

        if (number_components == 4)
        {
            jas_image_setcmpttype(jp2_image, 3, JAS_IMAGE_CT_OPACITY);
        }

        jas_image_setclrspc(jp2_image, JAS_CLRSPC_SRGB);
        jas_image_setcmpttype(jp2_image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
        jas_image_setcmpttype(jp2_image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
        jas_image_setcmpttype(jp2_image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
    }

    // -------------------------------------------------------------------
    // Set ICC color profile.

    // FIXME : doesn't work yet!

    QByteArray profile_rawdata = m_image->getIccProfile().data();

    if (!profile_rawdata.isEmpty())
    {
        jas_iccprof_t* const icc_profile = jas_iccprof_createfrombuf(reinterpret_cast<unsigned char*>(profile_rawdata.data()), profile_rawdata.size());

        if (icc_profile)
        {
            jas_cmprof_t* const cm_profile = jas_cmprof_createfromiccprof(icc_profile);

            if (cm_profile)
            {
                jas_image_setcmprof(jp2_image, cm_profile);

                // TODO: enable when it works
                // purgeExifWorkingColorSpace();
            }
        }
    }

    // workaround:

    storeColorProfileInMetadata();

    // -------------------------------------------------------------------
    // Convert to JPEG 2000 pixels.

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        pixels[i] = jas_matrix_create(1, (unsigned int)imageWidth());

        if (pixels[i] == nullptr)
        {
            for (x = 0 ; x < i ; ++x)
            {
                jas_matrix_destroy(pixels[x]);
            }

            qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error encoding JPEG2000 image data : Memory Allocation Failed";
            jas_image_destroy(jp2_image);

            cleanupJasper();

            return false;
        }
    }

    unsigned char*       data       = imageData();
    const unsigned char* pixel      = nullptr;
    unsigned short       r          = 0;
    unsigned short       g          = 0;
    unsigned short       b          = 0;
    unsigned short       a          = 0;
    uint                 checkpoint = 0;

    for (y = 0 ; y < (long)imageHeight() ; ++y)
    {
        if (observer && (y == (long)checkpoint))
        {
            checkpoint += granularity(observer, imageHeight(), 0.8F);

            if (!observer->continueQuery())
            {
                jas_image_destroy(jp2_image);

                for (i = 0 ; i < (long)number_components ; ++i)
                {
                    jas_matrix_destroy(pixels[i]);
                }

                cleanupJasper();

                return false;
            }

            observer->progressInfo(0.1F + (0.8F * (((float)y) / ((float)imageHeight()))));
        }

        for (x = 0 ; x < (long)imageWidth() ; ++x)
        {
            pixel = &data[((y * imageWidth()) + x) * imageBytesDepth()];

            if (imageSixteenBit())          // 16 bits image.
            {
                b = (unsigned short)(pixel[0] + 256 * pixel[1]);
                g = (unsigned short)(pixel[2] + 256 * pixel[3]);
                r = (unsigned short)(pixel[4] + 256 * pixel[5]);

                if (imageHasAlpha())
                {
                    a = (unsigned short)(pixel[6] + 256 * pixel[7]);
                }
            }
            else                            // 8 bits image.
            {
                b = (unsigned short)pixel[0];
                g = (unsigned short)pixel[1];
                r = (unsigned short)pixel[2];

                if (imageHasAlpha())
                {
                    a = (unsigned short)(pixel[3]);
                }
            }

            jas_matrix_setv(pixels[0], x, r);
            jas_matrix_setv(pixels[1], x, g);
            jas_matrix_setv(pixels[2], x, b);

            if (number_components > 3)
            {
                jas_matrix_setv(pixels[3], x, a);
            }
        }

        for (i = 0 ; i < (long)number_components ; ++i)
        {
            int ret = jas_image_writecmpt(jp2_image, (short) i, 0, (unsigned int)y,
                                          (unsigned int)imageWidth(), 1, pixels[i]);

            if (ret != 0)
            {
                qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Error encoding JPEG2000 image data";

                jas_image_destroy(jp2_image);

                for (i = 0 ; i < (long)number_components ; ++i)
                {
                    jas_matrix_destroy(pixels[i]);
                }

                cleanupJasper();

                return false;
            }
        }
    }

    QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
    int quality          = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    if (quality < 0)
    {
        quality = 90;
    }

    if (quality > 100)
    {
        quality = 100;
    }

    // optstr:
    // - rate=#B => the resulting file size is about # bytes
    // - rate=0.0 .. 1.0 => the resulting file size is about the factor times
    //                      the uncompressed size
    // use sprintf for locale-aware string

    char rateBuffer[16];
    snprintf(rateBuffer, sizeof(rateBuffer), "rate=%.2g", (quality / 100.0));

    qCDebug(DIGIKAM_DIMG_LOG_JP2K) << "JPEG2000 quality: " << quality;
    qCDebug(DIGIKAM_DIMG_LOG_JP2K) << "JPEG2000 "          << rateBuffer;

    int fmt = jas_image_strtofmt(QByteArray("jp2").data());
    int ret = jas_image_encode(jp2_image, jp2_stream, fmt, rateBuffer);

    if (ret != 0)
    {
        qCWarning(DIGIKAM_DIMG_LOG_JP2K) << "Unable to encode JPEG2000 image";

        jas_image_destroy(jp2_image);
        jas_stream_close(jp2_stream);

        for (i = 0 ; i < (long)number_components ; ++i)
        {
            jas_matrix_destroy(pixels[i]);
        }

        cleanupJasper();

        return false;
    }

    if (observer)
    {
        observer->progressInfo(1.0F);
    }

    jas_image_destroy(jp2_image);
    jas_stream_close(jp2_stream);

    for (i = 0 ; i < (long)number_components ; ++i)
    {
        jas_matrix_destroy(pixels[i]);
    }

    cleanupJasper();

    imageSetAttribute(QLatin1String("savedFormat"), QLatin1String("JP2"));
    saveMetadata(filePath);

    return true;
}

} // namespace DigikamJPEG2000DImgPlugin
