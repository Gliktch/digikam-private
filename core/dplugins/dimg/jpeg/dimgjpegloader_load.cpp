/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A JPEG IO file for DImg framework - load operations
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#define XMD_H

#include "dimgjpegloader.h"

// C ANSI includes

extern "C"
{
#include "iccjpeg.h"
}

// Qt includes

#include <QFile>
#include <QByteArray>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dimgloaderobserver.h"

#ifdef Q_OS_WIN
#   include "jpegwin.h"
#endif

namespace DigikamJPEGDImgPlugin
{

bool DImgJPEGLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath);

#ifdef Q_OS_WIN

    FILE* const file = _wfopen((const wchar_t*)filePath.utf16(), L"rb");

#else

    FILE* const file = fopen(filePath.toUtf8().constData(), "rb");

#endif

    if (!file)
    {
        loadingFailed();

        return false;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;

    // -------------------------------------------------------------------
    // JPEG error handling.

    cinfo.err                 = jpeg_std_error(&jerr);
    cinfo.err->error_exit     = dimg_jpeg_error_exit;
    cinfo.err->emit_message   = dimg_jpeg_emit_message;
    cinfo.err->output_message = dimg_jpeg_output_message;

    // try/catch cleanup

    class Q_DECL_HIDDEN CleanupData
    {
    public:

        CleanupData() = default;

        ~CleanupData()
        {
            delete [] data;
            delete [] dest;

            if (file)
            {
                fclose(file);
            }
        }

        void setData(uchar* const d)
        {
            data = d;
        }

        void setDest(uchar* const d)
        {
            dest = d;
        }

        void setFile(FILE* const f)
        {
            file = f;
        }

        void setSize(const QSize& s)
        {
            size = s;
        }

        void setColorModel(int c)
        {
            cmod = c;
        }

        void deleteData()
        {
            delete [] data;
            data = nullptr;
        }

        void takeDest()
        {
            dest = nullptr;
        }

    public:

        uchar* data = nullptr;
        uchar* dest = nullptr;
        FILE*  file = nullptr;

        QSize  size;
        int    cmod = 0;

    private:

        // Disable
        CleanupData(const CleanupData&)            = delete;
        CleanupData& operator=(const CleanupData&) = delete;
    };

    CleanupData* const cleanupData = new CleanupData;
    cleanupData->setFile(file);

    try
    {
        // -------------------------------------------------------------------
        // Find out if we do the fast-track loading with reduced size. Jpeg specific.

        int scaledLoadingSize = 0;
        QVariant attribute    = imageGetAttribute(QLatin1String("scaledLoadingSize"));

        if (attribute.isValid())
        {
            scaledLoadingSize = attribute.toInt();
        }

        // -------------------------------------------------------------------
        // Set JPEG decompressor instance

        jpeg_create_decompress(&cinfo);
        bool startedDecompress = false;

        jpeg_stdio_src(&cinfo, file);

        // Recording ICC profile marker (from iccjpeg.c)

        if (m_loadFlags & LoadICCData)
        {
            setup_read_icc_profile(&cinfo);
        }

        // read image information

        jpeg_read_header(&cinfo, boolean(true));

        // read dimension (nominal values from header)

        int w = cinfo.image_width;
        int h = cinfo.image_height;
        QSize originalSize(w, h);

        // Libjpeg handles the following conversions:
        // YCbCr => GRAYSCALE, YCbCr => RGB, GRAYSCALE => RGB, YCCK => CMYK
        // So we cannot get RGB from CMYK or YCCK, CMYK conversion is handled below

        int colorModel = DImg::COLORMODELUNKNOWN;

        switch (cinfo.jpeg_color_space)
        {
            case JCS_UNKNOWN:
            {
                // perhaps jpeg_read_header did some guessing, leave value unchanged

                colorModel            = DImg::COLORMODELUNKNOWN;
                break;
            }

            case JCS_GRAYSCALE:
            {
                cinfo.out_color_space = JCS_RGB;
                colorModel            = DImg::GRAYSCALE;
                break;
            }

            case JCS_RGB:
            {
                cinfo.out_color_space = JCS_RGB;
                colorModel            = DImg::RGB;
                break;
            }

            case JCS_YCbCr:
            {
                cinfo.out_color_space = JCS_RGB;
                colorModel            = DImg::YCBCR;
                break;
            }

            case JCS_CMYK:
            case JCS_YCCK:
            {
                cinfo.out_color_space = JCS_CMYK;
                colorModel            = DImg::CMYK;
                break;
            }

            default:
            {
                break;
            }
        }

        cleanupData->setColorModel(colorModel);

        // -------------------------------------------------------------------
        // Load image data.

        uchar* dest = nullptr;

        if (m_loadFlags & LoadImageData)
        {
            // set decompression parameters

            cinfo.do_fancy_upsampling = boolean(true);
            cinfo.do_block_smoothing  = boolean(false);

            // handle scaled loading

            if (scaledLoadingSize)
            {
                int imgSize = qMax(cinfo.image_width, cinfo.image_height);
                int scale   = 1;

                // libjpeg supports 1/1, 1/2, 1/4, 1/8

                while (scaledLoadingSize * scale * 2 <= imgSize)
                {
                    scale *= 2;
                }

                if (scale > 8)
                {
                    scale = 8;
                }
/*
                cinfo.scale_num   = 1;
                cinfo.scale_denom = scale;
*/
                cinfo.scale_denom *= scale;
            }

            // initialize decompression

            if (!startedDecompress)
            {
                jpeg_start_decompress(&cinfo);
                startedDecompress = true;
            }

            // some pseudo-progress

            if (observer)
            {
                observer->progressInfo(0.1F);
            }

            // re-read dimension (scaling included)

            w = cinfo.output_width;
            h = cinfo.output_height;

            // -------------------------------------------------------------------
            // Get scanlines

            uchar* ptr  = nullptr, *data = nullptr, *line[16];
            uchar* ptr2 = nullptr;
            int    x, y, l, i, scans;
            //        int count;
            //        int prevy;

            if (cinfo.rec_outbuf_height > 16)
            {
                jpeg_destroy_decompress(&cinfo);
                qCWarning(DIGIKAM_DIMG_LOG_JPEG) << "Height of JPEG scanline buffer out of range!";
                delete cleanupData;
                loadingFailed();

                return false;
            }

            // We only take RGB with 1 or 3 components, or CMYK with 4 components

            if (!(
                   (
                     (cinfo.out_color_space    == JCS_RGB)  &&
                     ((cinfo.output_components == 3) || (cinfo.output_components == 1))
                   ) ||
                   (
                     (cinfo.out_color_space    == JCS_CMYK) &&
                     (cinfo.output_components  == 4)
                   )
                 )
               )
            {
                jpeg_destroy_decompress(&cinfo);
                qCWarning(DIGIKAM_DIMG_LOG_JPEG)
                        << "JPEG colorspace ("
                        << cinfo.out_color_space
                        << ") or Number of JPEG color components ("
                        << cinfo.output_components
                        << ") unsupported!";
                delete cleanupData;
                loadingFailed();

                return false;
            }

            data = new_failureTolerant(w * 16 * cinfo.output_components);
            cleanupData->setData(data);

            if (!data)
            {
                jpeg_destroy_decompress(&cinfo);
                qCWarning(DIGIKAM_DIMG_LOG_JPEG) << "Cannot allocate memory!";
                delete cleanupData;
                loadingFailed();

                return false;
            }

            dest = new_failureTolerant(w, h, 4);
            cleanupData->setSize(QSize(w, h));
            cleanupData->setDest(dest);

            if (!dest)
            {
                jpeg_destroy_decompress(&cinfo);
                qCWarning(DIGIKAM_DIMG_LOG_JPEG) << "Cannot allocate memory!";
                delete cleanupData;
                loadingFailed();

                return false;
            }

            ptr2  = dest;
            //        count = 0;
            //        prevy = 0;

            if (cinfo.output_components == 3)
            {
                for (i = 0 ; i < cinfo.rec_outbuf_height ; ++i)
                {
                    line[i] = data + (i * w * 3);
                }

                int checkPoint = 0;

                for (l = 0 ; l < h ; l += cinfo.rec_outbuf_height)
                {
                    // use 0-10% and 90-100% for pseudo-progress

                    if (observer && (l >= checkPoint))
                    {
                        checkPoint += granularity(observer, h, 0.8F);

                        if (!observer->continueQuery())
                        {
                            jpeg_destroy_decompress(&cinfo);
                            delete cleanupData;
                            loadingFailed();

                            return false;
                        }

                        observer->progressInfo(0.1F + (0.8F * (((float)l) / ((float)h))));
                    }

                    jpeg_read_scanlines(&cinfo, &line[0], cinfo.rec_outbuf_height);
                    scans = cinfo.rec_outbuf_height;

                    if ((h - l) < scans)
                    {
                        scans = h - l;
                    }

                    ptr = data;

                    for (y = 0 ; y < scans ; ++y)
                    {
                        for (x = 0 ; x < w ; ++x)
                        {
                            ptr2[3] = 0xFF;
                            ptr2[2] = ptr[0];
                            ptr2[1] = ptr[1];
                            ptr2[0] = ptr[2];

                            ptr    += 3;
                            ptr2   += 4;
                        }
                    }
                }
            }
            else if (cinfo.output_components == 1)
            {
                for (i = 0 ; i < cinfo.rec_outbuf_height ; ++i)
                {
                    line[i] = data + (i * w);
                }

                int checkPoint = 0;

                for (l = 0 ; l < h ; l += cinfo.rec_outbuf_height)
                {
                    if (observer && (l >= checkPoint))
                    {
                        checkPoint += granularity(observer, h, 0.8F);

                        if (!observer->continueQuery())
                        {
                            jpeg_destroy_decompress(&cinfo);
                            delete cleanupData;
                            loadingFailed();

                            return false;
                        }

                        observer->progressInfo(0.1F + (0.8F * (((float)l) / ((float)h))));
                    }

                    jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
                    scans = cinfo.rec_outbuf_height;

                    if ((h - l) < scans)
                    {
                        scans = h - l;
                    }

                    ptr = data;

                    for (y = 0 ; y < scans ; ++y)
                    {
                        for (x = 0 ; x < w ; ++x)
                        {
                            ptr2[3] = 0xFF;
                            ptr2[2] = ptr[0];
                            ptr2[1] = ptr[0];
                            ptr2[0] = ptr[0];

                            ptr    ++;
                            ptr2   += 4;
                        }
                    }
                }
            }
            else // CMYK
            {
                for (i = 0 ; i < cinfo.rec_outbuf_height ; ++i)
                {
                    line[i] = data + (i * w * 4);
                }

                int checkPoint = 0;

                for (l = 0 ; l < h ; l += cinfo.rec_outbuf_height)
                {
                    // use 0-10% and 90-100% for pseudo-progress

                    if (observer && (l >= checkPoint))
                    {
                        checkPoint += granularity(observer, h, 0.8F);

                        if (!observer->continueQuery())
                        {
                            jpeg_destroy_decompress(&cinfo);
                            delete cleanupData;
                            loadingFailed();

                            return false;
                        }

                        observer->progressInfo(0.1F + (0.8F * (((float)l) / ((float)h))));
                    }

                    jpeg_read_scanlines(&cinfo, &line[0], cinfo.rec_outbuf_height);
                    scans = cinfo.rec_outbuf_height;

                    if ((h - l) < scans)
                    {
                        scans = h - l;
                    }

                    ptr   = data;

                    for (y = 0 ; y < scans ; ++y)
                    {
                        for (x = 0 ; x < w ; ++x)
                        {
                            // Inspired by Qt's JPEG loader

                            int k = ptr[3];

                            ptr2[3] = 0xFF;
                            ptr2[2] = k * ptr[0] / 255;
                            ptr2[1] = k * ptr[1] / 255;
                            ptr2[0] = k * ptr[2] / 255;

                            ptr    += 4;
                            ptr2   += 4;
                        }
                    }
                }
            }

            // clean up

            cleanupData->deleteData();
        }

        // -------------------------------------------------------------------
        // Read image ICC profile

        if (m_loadFlags & LoadICCData)
        {
            if (!startedDecompress)
            {
                jpeg_start_decompress(&cinfo);
                startedDecompress = true;
            }

            JOCTET* profile_data = nullptr;
            uint    profile_size = 0;

            read_icc_profile(&cinfo, &profile_data, &profile_size);

            if (profile_data != nullptr)
            {
                QByteArray profile_rawdata;
                profile_rawdata.resize(profile_size);
                memcpy(profile_rawdata.data(), profile_data, profile_size);
                imageSetIccProfile(IccProfile(profile_rawdata));
                free(profile_data);
            }
            else
            {
                // If ICC profile is null, check Exif metadata.

                checkExifWorkingColorSpace();
            }
        }

        // -------------------------------------------------------------------

        if (startedDecompress)
        {
            jpeg_finish_decompress(&cinfo);
        }

        jpeg_destroy_decompress(&cinfo);

        // -------------------------------------------------------------------

        cleanupData->takeDest();
        delete cleanupData;

        if (observer)
        {
            observer->progressInfo(1.0F);
        }

        imageWidth()  = w;
        imageHeight() = h;
        imageData()   = dest;
        imageSetAttribute(QLatin1String("format"),             QLatin1String("JPG"));
        imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
        imageSetAttribute(QLatin1String("originalBitDepth"),   8);
        imageSetAttribute(QLatin1String("originalSize"),       originalSize);

        return true;
    }
    catch (std::runtime_error&)
    {
        jpeg_destroy_decompress(&cinfo);

        if (!cleanupData->dest ||
            !cleanupData->size.isValid())
        {
            delete cleanupData;
            loadingFailed();

            return false;
        }

        // We check only Exif metadata for ICC profile to prevent endless loop

        if (m_loadFlags & LoadICCData)
        {
            checkExifWorkingColorSpace();
        }

        if (observer)
        {
            observer->progressInfo(1.0F);
        }

        imageWidth()  = cleanupData->size.width();
        imageHeight() = cleanupData->size.height();
        imageData()   = cleanupData->dest;
        imageSetAttribute(QLatin1String("format"),             QLatin1String("JPG"));
        imageSetAttribute(QLatin1String("originalColorModel"), cleanupData->cmod);
        imageSetAttribute(QLatin1String("originalBitDepth"),   8);
        imageSetAttribute(QLatin1String("originalSize"),       cleanupData->size);

        cleanupData->takeDest();
        delete cleanupData;

        return true;
    }
}

} // namespace DigikamJPEGDImgPlugin
