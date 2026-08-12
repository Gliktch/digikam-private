/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : a PNG image loader for DImg framework - save operations.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpngloader.h"

// C ANSI includes

extern "C"
{
#ifndef Q_CC_MSVC
#   include <unistd.h>
#endif
}

// C++ includes

#include <cstdlib>
#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>
#include <QSysInfo>

// Local includes

#include "metaengine.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"
#include "dimgloaderobserver.h"

// libPNG includes

extern "C"
{
#include <png.h>
}

using namespace Digikam;

namespace DigikamPNGDImgPlugin
{

#if PNG_LIBPNG_VER_MAJOR >= 1 && PNG_LIBPNG_VER_MINOR >= 5

typedef png_bytep iCCP_data;

#else

typedef png_charp iCCP_data;

#endif

bool DImgPNGLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    png_structp    png_ptr;
    png_infop      info_ptr;
    uint           x, y, j;
    png_bytep      row_ptr;
    png_color_8    sig_bit;
    FILE*          f           = nullptr;
    uchar*         ptr         = nullptr;
    uchar*         data        = nullptr;
    int            quality     = 75;
    int            compression = 3;

    // Tp prevent cppcheck warnings.
    (void)f;
    (void)ptr;
    (void)data;
    (void)quality;
    (void)compression;

    // -------------------------------------------------------------------
    // Open the file

#ifdef Q_OS_WIN

    f = _wfopen((const wchar_t*)filePath.utf16(), L"wb");

#else

    f = fopen(filePath.toUtf8().constData(), "wb");

#endif

    if (!f)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot open target image file.";

        return false;
    }

    // -------------------------------------------------------------------
    // Initialize the internal structures

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Invalid target PNG image file structure.";
        fclose(f);

        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == nullptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot create PNG image file structure.";
        png_destroy_write_struct(&png_ptr, (png_infopp) nullptr);
        fclose(f);

        return false;
    }

    // -------------------------------------------------------------------
    // PNG error handling. If an error occurs during writing, libpng
    // will jump here

    // setjmp-save cleanup

    class Q_DECL_HIDDEN CleanupData
    {
    public:

        CleanupData() = default;

        ~CleanupData()
        {
            delete [] data;

            if (f)
            {
                fclose(f);
            }
        }

        void setData(uchar* const d)
        {
            data = d;
        }

        void setFile(FILE* const file)
        {
            f = file;
        }

        uchar* data = nullptr;
        FILE*  f    = nullptr;

    private:

        // Disable
        CleanupData(const CleanupData&)            = delete;
        CleanupData& operator=(const CleanupData&) = delete;
    };

    CleanupData* const cleanupData = new CleanupData;
    cleanupData->setFile(f);

#if PNG_LIBPNG_VER >= 10400

    if (setjmp(png_jmpbuf(png_ptr)))

#else

    if (setjmp(png_ptr->jmpbuf))

#endif

    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Internal libPNG error during writing file. Process aborted!";
        png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
        png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
        delete cleanupData;

        return false;
    }

#ifdef PNG_BENIGN_ERRORS_SUPPORTED

  // Change some libpng errors to warnings (e.g. bug 386396).

  png_set_benign_errors(png_ptr, true);

  png_set_option(png_ptr, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);

#endif

    png_init_io(png_ptr, f);
    png_set_bgr(png_ptr);

    //png_set_swap_alpha(png_ptr);

    if (imageHasAlpha())
    {
        png_set_IHDR(png_ptr, info_ptr, imageWidth(), imageHeight(), imageBitsDepth(),
                     PNG_COLOR_TYPE_RGB_ALPHA,  PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        if (imageSixteenBit())
        {
            data = new uchar[imageWidth() * 8 * sizeof(uchar)] { 0 };
        }
        else
        {
            data = new uchar[imageWidth() * 4 * sizeof(uchar)] { 0 };
        }
    }
    else
    {
        png_set_IHDR(png_ptr, info_ptr, imageWidth(), imageHeight(), imageBitsDepth(),
                     PNG_COLOR_TYPE_RGB,        PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        if (imageSixteenBit())
        {
            data = new uchar[imageWidth() * 6 * sizeof(uchar)] { 0 };
        }
        else
        {
            data = new uchar[imageWidth() * 3 * sizeof(uchar)] { 0 };
        }
    }

    cleanupData->setData(data);

    sig_bit.red   = imageBitsDepth();
    sig_bit.green = imageBitsDepth();
    sig_bit.blue  = imageBitsDepth();
    sig_bit.alpha = imageBitsDepth();
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);

    // -------------------------------------------------------------------
    // Quality to convert to compression

    QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
    quality              = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "DImg quality level: " << quality;

    if (quality < 1)
    {
        quality = 1;
    }

    if (quality > 99)
    {
        quality = 99;
    }

    quality     = quality / 10;
    compression = 9 - quality;

    if (compression < 0)
    {
        compression = 0;
    }

    if (compression > 9)
    {
        compression = 9;
    }

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG compression level: " << compression;
    png_set_compression_level(png_ptr, compression);

    // -------------------------------------------------------------------
    // Write ICC profile.

    QByteArray profile_rawdata = m_image->getIccProfile().data();

    if (!profile_rawdata.isEmpty())
    {
        purgeExifWorkingColorSpace();
        png_set_iCCP(png_ptr, info_ptr, (png_charp)("icc"),
                     PNG_COMPRESSION_TYPE_BASE,
                     (iCCP_data)profile_rawdata.data(),
                     profile_rawdata.size());
    }

    // -------------------------------------------------------------------
    // Write embedded Text

    typedef QMap<QString, QString> EmbeddedTextMap;
    EmbeddedTextMap map = imageEmbeddedText();

    for (EmbeddedTextMap::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        if ((it.key() != QLatin1String("Software")) && (it.key() != QLatin1String("Comment")))
        {
            QByteArray key   = it.key().toLatin1();
            QByteArray value = it.value().toLatin1();
            png_text text;
            text.key         = key.data();
            text.text        = value.data();

            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Writing PNG Embedded text: key="
                                          << text.key << " text=" << text.text;

            text.compression = PNG_TEXT_COMPRESSION_zTXt;
            png_set_text(png_ptr, info_ptr, &(text), 1);
        }
    }

    // Update 'Software' text tag.

    QString software  = QLatin1String("digiKam ");
    software.append(digiKamVersion());
    QString libpngver = QLatin1String(PNG_HEADER_VERSION_STRING);
    libpngver.replace(QLatin1Char('\n'), QLatin1Char(' '));
    software.append(QString::fromLatin1(" (%1)").arg(libpngver));
    QByteArray softwareAsAscii = software.toLatin1();
    png_text text;
    text.key  = (png_charp)("Software");
    text.text = softwareAsAscii.data();

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Writing PNG Embedded text: key="
                                  << text.key << " text=" << text.text;

    text.compression = PNG_TEXT_COMPRESSION_zTXt;
    png_set_text(png_ptr, info_ptr, &(text), 1);

    if (observer)
    {
        observer->progressInfo(0.2F);
    }

    // -------------------------------------------------------------------
    // Write image data

    png_write_info(png_ptr, info_ptr);
    png_set_shift(png_ptr, &sig_bit);
    png_set_packing(png_ptr);
    ptr = imageData();

    uint checkPoint = 0;

    for (y = 0 ; y < imageHeight() ; ++y)
    {

        if (observer && (y == checkPoint))
        {
            checkPoint += granularity(observer, imageHeight(), 0.8F);

            if (!observer->continueQuery())
            {
                png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
                png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
                delete cleanupData;

                return false;
            }

            observer->progressInfo(0.2F + (0.8F * (((float)y) / ((float)imageHeight()))));
        }

        j = 0;

        if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        {
            for (x = 0 ; x < imageWidth() * imageBytesDepth() ; x += imageBytesDepth())
            {
                if (imageSixteenBit())
                {
                    if (imageHasAlpha())
                    {
                        data[j++] = ptr[x + 1]; // Blue
                        data[j++] = ptr[  x  ];
                        data[j++] = ptr[x + 3]; // Green
                        data[j++] = ptr[x + 2];
                        data[j++] = ptr[x + 5]; // Red
                        data[j++] = ptr[x + 4];
                        data[j++] = ptr[x + 7]; // Alpha
                        data[j++] = ptr[x + 6];
                    }
                    else
                    {
                        data[j++] = ptr[x + 1]; // Blue
                        data[j++] = ptr[  x  ];
                        data[j++] = ptr[x + 3]; // Green
                        data[j++] = ptr[x + 2];
                        data[j++] = ptr[x + 5]; // Red
                        data[j++] = ptr[x + 4];
                    }
                }
                else
                {
                    if (imageHasAlpha())
                    {
                        data[j++] = ptr[  x  ]; // Blue
                        data[j++] = ptr[x + 1]; // Green
                        data[j++] = ptr[x + 2]; // Red
                        data[j++] = ptr[x + 3]; // Alpha
                    }
                    else
                    {
                        data[j++] = ptr[  x  ]; // Blue
                        data[j++] = ptr[x + 1]; // Green
                        data[j++] = ptr[x + 2]; // Red
                    }
                }
            }
        }
        else
        {
            int bytes = (imageSixteenBit() ? 2 : 1) * (imageHasAlpha() ? 4 : 3);

            for (x = 0 ; x < imageWidth() * imageBytesDepth() ; x += imageBytesDepth())
            {
                memcpy(data + j, ptr + x, bytes);
                j += bytes;
            }
        }

        row_ptr = (png_bytep) data;

        png_write_rows(png_ptr, &row_ptr, 1);
        ptr += (imageWidth() * imageBytesDepth());
    }

    // -------------------------------------------------------------------

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
    png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);

    delete cleanupData;

    imageSetAttribute(QLatin1String("savedFormat"), QLatin1String("PNG"));

    saveMetadata(filePath);

    return true;
}

} // namespace DigikamPNGDImgPlugin
