/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A JPEG IO file for DImg framework - save operations
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

bool DImgJPEGLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
#ifdef Q_OS_WIN

    FILE* const file = _wfopen((const wchar_t*)filePath.utf16(), L"wb");

#else

    FILE* const file = fopen(filePath.toUtf8().constData(), "wb");

#endif

    if (!file)
    {
        return false;
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

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
            deleteLine();

            if (f)
            {
                fclose(f);
            }
        }

        void setLine(uchar* const l)
        {
            line = l;
        }

        void setFile(FILE* const _file)
        {
            f = _file;
        }

        void deleteLine()
        {
            delete [] line;
            line = nullptr;
        }

    public:

        uchar* line = nullptr;
        FILE*  f    = nullptr;

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
        // Set JPEG compressor instance

        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, file);

        const uint&        w   = imageWidth();
        const uint&        h   = imageHeight();
        unsigned char*& data   = imageData();

        // Size of image.

        cinfo.image_width      = w;
        cinfo.image_height     = h;

        // Color components of image in RGB.

        cinfo.input_components = 3;
        cinfo.in_color_space   = JCS_RGB;

        QVariant qualityAttr   = imageGetAttribute(QLatin1String("quality"));
        int quality            = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

        if (quality < 0)
        {
            quality = 90;
        }

        if (quality > 100)
        {
            quality = 100;
        }

        QVariant subSamplingAttr = imageGetAttribute(QLatin1String("subsampling"));
        int subsampling          = subSamplingAttr.isValid() ? subSamplingAttr.toInt() : 1;  // Medium

        jpeg_set_defaults(&cinfo);

        // bug #149578: set horizontal and vertical chroma subsampling factor to encoder.
        // See this page for details: https://en.wikipedia.org/wiki/Chroma_subsampling

        switch (subsampling)
        {
            case 1:  // 2x1, 1x1, 1x1 (4:2:2)
            {
                qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:2:2";
                cinfo.comp_info[0].h_samp_factor = 2;
                cinfo.comp_info[0].v_samp_factor = 1;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }

            case 2:  // 2x2, 1x1, 1x1 (4:2:0)
            {
                qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:2:0";
                cinfo.comp_info[0].h_samp_factor = 2;
                cinfo.comp_info[0].v_samp_factor = 2;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }

            case 3:  // 4x1, 1x1, 1x1 (4:1:1)
            {
                qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:1:1";
                cinfo.comp_info[0].h_samp_factor = 4;
                cinfo.comp_info[0].v_samp_factor = 1;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }

            default: // 1x1, 1x1, 1x1 (4:4:4)
            {
                qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:4:4";
                cinfo.comp_info[0].h_samp_factor = 1;
                cinfo.comp_info[0].v_samp_factor = 1;
                cinfo.comp_info[1].h_samp_factor = 1;
                cinfo.comp_info[1].v_samp_factor = 1;
                cinfo.comp_info[2].h_samp_factor = 1;
                cinfo.comp_info[2].v_samp_factor = 1;
                break;
            }
        }

        jpeg_set_quality(&cinfo, quality, boolean(true));
        jpeg_start_compress(&cinfo, boolean(true));

        qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG quality compression value: " << quality;

        if (observer)
        {
            observer->progressInfo(0.1F);
        }

        // -------------------------------------------------------------------
        // Write ICC profile.

        QByteArray profile_rawdata = m_image->getIccProfile().data();

        if (!profile_rawdata.isEmpty())
        {
            purgeExifWorkingColorSpace();
            write_icc_profile(&cinfo, reinterpret_cast<JOCTET*>(profile_rawdata.data()), profile_rawdata.size());
        }

        if (observer)
        {
            observer->progressInfo(0.2F);
        }

        // -------------------------------------------------------------------
        // Write Image data.

        uchar* line       = new uchar[w * 3] { 0 };
        uchar* dstPtr     = nullptr;
        uint   checkPoint = 0;
        cleanupData->setLine(line);

        if (!imageSixteenBit())     // 8 bits image.
        {
            uchar* srcPtr = data;

            for (uint j = 0 ; j < h ; ++j)
            {

                if (observer && j == checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery())
                    {
                        jpeg_destroy_compress(&cinfo);
                        delete cleanupData;

                        return false;
                    }

                    // use 0-20% for pseudo-progress, now fill 20-100%

                    observer->progressInfo(0.2F + (0.8F * (((float)j) / ((float)h))));
                }

                dstPtr = line;

                for (uint i = 0 ; i < w ; ++i)
                {
                    dstPtr[2] = srcPtr[0];  // Blue
                    dstPtr[1] = srcPtr[1];  // Green
                    dstPtr[0] = srcPtr[2];  // Red

                    srcPtr   += 4;
                    dstPtr   += 3;
                }

                jpeg_write_scanlines(&cinfo, &line, 1);
            }
        }
        else
        {
            unsigned short* srcPtr = reinterpret_cast<unsigned short*>(data);

            for (uint j = 0 ; j < h ; ++j)
            {

                if (observer && j == checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery())
                    {
                        jpeg_destroy_compress(&cinfo);
                        delete cleanupData;

                        return false;
                    }

                    // use 0-20% for pseudo-progress, now fill 20-100%

                    observer->progressInfo(0.2F + (0.8F * (((float)j) / ((float)h))));
                }

                dstPtr = line;

                for (uint i = 0 ; i < w ; ++i)
                {
                    dstPtr[2] = (srcPtr[0] * 255UL) / 65535UL;  // Blue
                    dstPtr[1] = (srcPtr[1] * 255UL) / 65535UL;  // Green
                    dstPtr[0] = (srcPtr[2] * 255UL) / 65535UL;  // Red

                    srcPtr   += 4;
                    dstPtr   += 3;
                }

                jpeg_write_scanlines(&cinfo, &line, 1);
            }
        }

        cleanupData->deleteLine();

        // -------------------------------------------------------------------

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        delete cleanupData;

        imageSetAttribute(QLatin1String("savedFormat"), QLatin1String("JPG"));

        saveMetadata(filePath);

        return true;
    }
    catch (std::runtime_error&)
    {
        jpeg_destroy_compress(&cinfo);
        delete cleanupData;

        return false;
    }
}

} // namespace DigikamJPEGDImgPlugin
