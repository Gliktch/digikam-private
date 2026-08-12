/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-26
 * Description : A HEIF IO file for DImg framework - save operations
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgheifloader.h"

// Qt includes

#include <QVariant>
#include <QByteArray>
#include <QElapsedTimer>
#include <qplatformdefs.h>
#include <QScopedPointer>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "metaengine.h"

// libx265 includes

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wundef"
#   pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#   pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

#ifdef HAVE_X265
#   include <x265.h>
#endif

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

namespace Digikam
{

int DImgHEIFLoader::x265MaxBitsDepth()
{
    int maxOutputBitsDepth = -1;

#ifdef HAVE_X265

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEVC encoder max bit depth:" << x265_max_bit_depth;
    const x265_api* api = x265_api_get(x265_max_bit_depth);

    if (api)
    {
        maxOutputBitsDepth = x265_max_bit_depth;
        qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEVC encoder max bits depth:" << maxOutputBitsDepth;
    }
    else
    {
        api = x265_api_get(8); // Try to failback to default 8 bits

        if (api)
        {
            maxOutputBitsDepth = 8;
            qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEVC encoder max bits depth: 8 (default failback value)";
        }
    }

#endif

    // cppcheck-suppress knownConditionTrueFalse
    if (maxOutputBitsDepth == -1)
    {
        qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Cannot get max supported HEVC encoder bits depth!";
    }

    return maxOutputBitsDepth;
}

bool DImgHEIFLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    m_observer             = observer;

    QVariant qualityAttr   = imageGetAttribute(QLatin1String("quality"));
    int quality            = qualityAttr.isValid() ? qualityAttr.toInt() : 75;
    bool lossless          = (quality == 0);

    // --- Determine libx265 encoder bits depth capability: 8=standard, 10, 12, or later 16.

    int maxOutputBitsDepth = x265MaxBitsDepth();

    if (maxOutputBitsDepth == -1)
    {
        return false;
    }

    heif_chroma chroma;

    if (maxOutputBitsDepth > 8)          // 16 bits image.
    {
        chroma = imageHasAlpha() ? heif_chroma_interleaved_RRGGBBAA_BE
                                 : heif_chroma_interleaved_RRGGBB_BE;
    }
    else
    {
        chroma = imageHasAlpha() ? heif_chroma_interleaved_RGBA
                                 : heif_chroma_interleaved_RGB;
    }

    // --- use standard HEVC encoder

    QElapsedTimer timer;
    timer.start();

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEVC encoder setup...";

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

    heif_init(nullptr);

#endif

    struct heif_context* const ctx = heif_context_alloc();

    if (!ctx)
    {
        qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Cannot create HEIF context!";

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    struct heif_image* image = nullptr;
    struct heif_error error  = heif_image_create(imageWidth(),
                                                 imageHeight(),
                                                 heif_colorspace_RGB,
                                                 chroma,
                                                 &image);

    if (!isHeifSuccess(&error))
    {
        heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    struct heif_encoder* encoder = nullptr;
    error                        = heif_context_get_encoder_for_format(ctx,
                                                                       heif_compression_HEVC,
                                                                       &encoder);

    if (!isHeifSuccess(&error))
    {
        heif_image_release(image);
        heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    heif_encoder_set_lossy_quality(encoder, quality);
    heif_encoder_set_lossless(encoder, lossless);

    // --- Save color profile before to create image data, as converting to color space can be processed at this stage.

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF set color profile...";

    saveHEICColorProfile(image);

    // --- Add image data

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF setup data plane...";

    error = heif_image_add_plane(image,
                                 heif_channel_interleaved,
                                 imageWidth(),
                                 imageHeight(),
                                 maxOutputBitsDepth);

    if (!isHeifSuccess(&error))
    {
        heif_encoder_release(encoder);
        heif_image_release(image);
        heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    int stride          = 0;
    uint8_t* const data = heif_image_get_plane(image,
                                               heif_channel_interleaved,
                                               &stride);

    if (!data || (stride <= 0))
    {
        qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "HEIF data pixels information not valid!";

        heif_encoder_release(encoder);
        heif_image_release(image);
        heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF data container:" << data;
    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF bytes per line:" << stride;

    uint checkpoint           = 0;
    unsigned char r           = 0;
    unsigned char g           = 0;
    unsigned char b           = 0;
    unsigned char a           = 0;
    unsigned char* src        = nullptr;
    unsigned char* dst        = nullptr;
    unsigned short r16        = 0;
    unsigned short g16        = 0;
    unsigned short b16        = 0;
    unsigned short a16        = 0;
    unsigned short* src16     = nullptr;
    unsigned short* dst16     = nullptr;
    int div16                 = 16 - maxOutputBitsDepth;
    int mul8                  = maxOutputBitsDepth - 8;
    int nbOutputBytesPerColor = (maxOutputBitsDepth > 8) ? (imageHasAlpha() ? 4 * 2 : 3 * 2)  // output data stored on 16 bits
                                                         : (imageHasAlpha() ? 4     : 3    ); // output data stored on 8 bits

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF output bytes per color:" << nbOutputBytesPerColor;
    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF 16 to 8 bits coeff.   :" << div16;
    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF 8 to 16 bits coeff.   :" << mul8;

    for (unsigned int y = 0 ; y < imageHeight() ; ++y)
    {
        src   = &imageData()[(y * imageWidth()) * imageBytesDepth()];
        src16 = reinterpret_cast<unsigned short*>(src);
        dst   = reinterpret_cast<unsigned char*>(data + (y * stride));
        dst16 = reinterpret_cast<unsigned short*>(dst);

        for (unsigned int x = 0 ; x < imageWidth() ; ++x)
        {
            if (imageSixteenBit())          // 16 bits source image.
            {
                b16 = src16[0];
                g16 = src16[1];
                r16 = src16[2];

                if (imageHasAlpha())
                {
                    a16 = src16[3];
                }

                if (maxOutputBitsDepth > 8) // From 16 bits to 10 bits or more.
                {
                    dst16[0] = (unsigned short)(r16 >> div16);
                    dst16[1] = (unsigned short)(g16 >> div16);
                    dst16[2] = (unsigned short)(b16 >> div16);

                    if (imageHasAlpha())
                    {
                        dst16[3] = (unsigned short)(a16 >> div16);
                        dst16   += 4;
                    }
                    else
                    {
                        dst16 += 3;
                    }
                }
                else                        // From 16 bits to 8 bits.
                {
                    dst[0] = (unsigned char)(r16 >> div16);
                    dst[1] = (unsigned char)(g16 >> div16);
                    dst[2] = (unsigned char)(b16 >> div16);

                    if (imageHasAlpha())
                    {
                        dst[3] = (unsigned char)(a16 >> div16);
                        dst   += 4;
                    }
                    else
                    {
                        dst += 3;
                    }
                }

                src16 += 4;
            }
            else                            // 8 bits source image.
            {
                b = src[0];
                g = src[1];
                r = src[2];

                if (imageHasAlpha())
                {
                    a = src[3];
                }

                if (maxOutputBitsDepth > 8) // From 8 bits to 10 bits or more.
                {
                    dst16[0] = (unsigned short)(r << mul8);
                    dst16[1] = (unsigned short)(g << mul8);
                    dst16[2] = (unsigned short)(b << mul8);

                    if (imageHasAlpha())
                    {
                        dst16[3] = (unsigned short)(a << mul8);
                        dst16   += 4;
                    }
                    else
                    {
                        dst16 += 3;
                    }
                }
                else                        // From 8 bits to 8 bits.
                {
                    dst[0] = r;
                    dst[1] = g;
                    dst[2] = b;

                    if (imageHasAlpha())
                    {
                        dst[3] = a;
                        dst   += 4;
                    }
                    else
                    {
                        dst += 3;
                    }
                }

                src += 4;
            }
        }

        if (m_observer && (y == (unsigned int)checkpoint))
        {
            checkpoint += granularity(m_observer, imageHeight(), 0.8F);

            if (!m_observer->continueQuery())
            {
                heif_encoder_release(encoder);
                heif_image_release(image);
                heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

                heif_deinit();

#endif
                return false;
            }

            m_observer->progressInfo(0.1F + (0.8F * (((float)y) / ((float)imageHeight()))));
        }
    }

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF master image encoding...";

    // --- encode and write master image

    struct heif_encoding_options* const options = heif_encoding_options_alloc();
    options->save_alpha_channel                 = imageHasAlpha() ? 1 : 0;
    struct heif_image_handle* image_handle      = nullptr;
    error                                       = heif_context_encode_image(ctx,
                                                                            image,
                                                                            encoder,
                                                                            options,
                                                                            &image_handle);

    if (!isHeifSuccess(&error))
    {
        heif_encoding_options_free(options);
        heif_encoder_release(encoder);
        heif_image_release(image);
        heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    // --- encode thumbnail

    // Note: Only encode preview for large image.
    // We will use the same preview size than DImg::prepareMetadataToSave()

    const int previewSize = 1280;

    if (qMin(imageWidth(), imageHeight()) > previewSize)
    {
        qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF preview storage in thumbnail chunk...";

        struct heif_image_handle* thumbnail_handle = nullptr;

        error = heif_context_encode_thumbnail(ctx,
                                              image,
                                              image_handle,
                                              encoder,
                                              options,
                                              previewSize,
                                              &thumbnail_handle);
        if (!isHeifSuccess(&error))
        {
            heif_image_handle_release(image_handle);
            heif_encoding_options_free(options);
            heif_encoder_release(encoder);
            heif_image_release(image);
            heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

            heif_deinit();

#endif

            return false;
        }

        heif_image_handle_release(thumbnail_handle);
    }

    // --- Add Exif and XMP metadata

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF metadata storage...";

    saveHEICMetadata(ctx, image_handle);

    // --- write HEIF file

    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF flush to file...";
    qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "HEIF encoding took:" << timer.elapsed() << "ms";

    error = heif_context_write_to_file(ctx, filePath.toUtf8().constData());

    if (!isHeifSuccess(&error))
    {
        heif_image_handle_release(image_handle);
        heif_encoding_options_free(options);
        heif_encoder_release(encoder);
        heif_image_release(image);
        heif_context_free(ctx);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

        heif_deinit();

#endif

        return false;
    }

    heif_image_handle_release(image_handle);
    heif_encoding_options_free(options);
    heif_encoder_release(encoder);
    heif_image_release(image);
    heif_context_free(ctx);

    imageSetAttribute(QLatin1String("savedFormat"), QLatin1String("HEIF"));
    saveMetadata(filePath);

#if LIBHEIF_NUMERIC_VERSION >= 0x010d0000

    heif_deinit();

#endif

    return true;
}

bool DImgHEIFLoader::saveHEICColorProfile(struct heif_image* const image) // cppcheck-suppress constParameterPointer
{

#if LIBHEIF_NUMERIC_VERSION >= 0x01040000

    QByteArray profile = m_image->getIccProfile().data();

    if (!profile.isEmpty())
    {
        // Save color profile.

        struct heif_error error = heif_image_set_raw_color_profile(image,
                                                                   "prof",           // FIXME: detect string in profile data
                                                                   profile.data(),
                                                                   profile.size());

        if (error.code != 0)
        {
            qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Cannot set HEIF color profile!";
            return false;
        }

        qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "Stored HEIF color profile size:" << profile.size();
    }

#else

    Q_UNUSED(image_handle);

#endif

    return true;
}

bool DImgHEIFLoader::saveHEICMetadata(struct heif_context* const heif_context,
                                      struct heif_image_handle* const image_handle)
{
    QScopedPointer<MetaEngine> meta(new MetaEngine(m_image->getMetadata()));

    if (!meta->hasExif() && !meta->hasIptc() && !meta->hasXmp())
    {
        return false;
    }

    QByteArray exif = meta->getExifEncoded();
    QByteArray iptc = meta->getIptc();
    QByteArray xmp  = meta->getXmp();
    struct heif_error error;

    if (!exif.isEmpty())
    {
        error = heif_context_add_exif_metadata(heif_context,
                                               image_handle,
                                               exif.data(),
                                               exif.size());

        if (error.code != 0)
        {
            qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Cannot store HEIF Exif metadata!";
            return false;
        }

        qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "Stored HEIF Exif data size:" << exif.size();
    }

    if (!iptc.isEmpty())
    {
        error = heif_context_add_generic_metadata(heif_context,
                                                  image_handle,
                                                  iptc.data(),
                                                  iptc.size(),
                                                  "iptc",
                                                  nullptr);

        if (error.code != 0)
        {
            qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Cannot store HEIF Iptc metadata!";

            return false;
        }

        qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "Stored HEIF Iptc data size:" << iptc.size();
    }

    if (!xmp.isEmpty())
    {
        error = heif_context_add_XMP_metadata(heif_context,
                                              image_handle,
                                              xmp.data(),
                                              xmp.size());

        if (error.code != 0)
        {
            qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Cannot store HEIF Xmp metadata!";

            return false;
        }

        qCDebug(DIGIKAM_DIMG_LOG_HEIF) << "Stored HEIF Xmp data size:" << xmp.size();
    }

    return true;
}

} // namespace Digikam
