/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : a PNG image loader for DImg framework - load operations.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#define PNG_BYTES_TO_CHECK 4

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

#ifdef Q_OS_WIN

void _ReadProc(struct png_struct_def* png_ptr, png_bytep data, png_size_t size)
{
    FILE* const file_handle = (FILE*)png_get_io_ptr(png_ptr);
    fread(data, size, 1, file_handle);
}

#endif

using namespace Digikam;

namespace DigikamPNGDImgPlugin
{

#if PNG_LIBPNG_VER_MAJOR >= 1 && PNG_LIBPNG_VER_MINOR >= 5

typedef png_bytep iCCP_data;

#else

typedef png_charp iCCP_data;

#endif

bool DImgPNGLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    png_uint_32  w32            = 0;
    png_uint_32  h32            = 0;
    int          width          = 0;
    int          height         = 0;
    int          bit_depth      = 0;
    int          color_type     = 0;
    int          interlace_type = 0;
    FILE*        f              = nullptr;
    png_structp  png_ptr        = nullptr;
    png_infop    info_ptr       = nullptr;

    // To prevent cppcheck warnings.
    (void)f;
    (void)png_ptr;
    (void)info_ptr;

    readMetadata(filePath);

    // -------------------------------------------------------------------
    // Open the file

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Opening file" << filePath;

#ifdef Q_OS_WIN

    f = _wfopen((const wchar_t*)filePath.utf16(), L"rb");

#else

    f = fopen(filePath.toUtf8().constData(), "rb");

#endif

    if (!f)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot open image file.";
        loadingFailed();

        return false;
    }

    unsigned char buf[PNG_BYTES_TO_CHECK];

    size_t membersRead = fread(buf, 1, PNG_BYTES_TO_CHECK, f);

#if PNG_LIBPNG_VER >= 10400

    if ((membersRead != PNG_BYTES_TO_CHECK) || png_sig_cmp(buf, 0, PNG_BYTES_TO_CHECK))

#else

    if ((membersRead != PNG_BYTES_TO_CHECK) || !png_check_sig(buf, PNG_BYTES_TO_CHECK))

#endif

    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Not a PNG image file.";
        fclose(f);
        loadingFailed();

        return false;
    }

    rewind(f);

    // -------------------------------------------------------------------
    // Initialize the internal structures

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Invalid PNG image file structure.";
        fclose(f);
        loadingFailed();

        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot reading PNG image file structure.";
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        fclose(f);
        loadingFailed();

        return false;
    }

    // -------------------------------------------------------------------
    // PNG error handling. If an error occurs during reading, libpng
    // will jump here

    // setjmp-save cleanup

    class Q_DECL_HIDDEN CleanupData
    {

    public:

        CleanupData() = default;

        ~CleanupData()
        {
            delete [] data;
            freeLines();

            if (file)
            {
                fclose(file);
            }
        }

        void setData(uchar* const d)
        {
            data = d;
        }

        void setLines(uchar** const l)
        {
            lines = l;
        }

        void setFile(FILE* const _f)
        {
            file = _f;
        }

        void setSize(const QSize& s)
        {
            size = s;
        }

        void setColorModel(int c)
        {
            cmod = c;
        }

        void takeData()
        {
            data = nullptr;
        }

        void freeLines()
        {
            if (lines)
            {
                free(lines);
            }

            lines = nullptr;
        }

    public:

        uchar*  data    = nullptr;
        uchar** lines   = { nullptr };
        FILE*   file    = nullptr;

        QSize   size;
        int     cmod    = 0;


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
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);

        if (
            !cleanupData->data ||
            !cleanupData->size.isValid()
           )
        {
            qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Internal libPNG error during reading file. Process aborted!";
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
        imageData()   = cleanupData->data;
        imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
        imageSetAttribute(QLatin1String("originalColorModel"), cleanupData->cmod);
        imageSetAttribute(QLatin1String("originalBitDepth"),   m_sixteenBit ? 16 : 8);
        imageSetAttribute(QLatin1String("originalSize"),       cleanupData->size);

        cleanupData->takeData();
        delete cleanupData;

        return true;
    }

#ifdef PNG_BENIGN_ERRORS_SUPPORTED

    // Change some libpng errors to warnings (e.g. bug 386396).

    png_set_benign_errors(png_ptr, true);

    png_set_option(png_ptr, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);

#endif

#ifdef Q_OS_WIN

    png_set_read_fn(png_ptr, f, _ReadProc);

#else

    png_init_io(png_ptr, f);

#endif

    // -------------------------------------------------------------------
    // Read all PNG info up to image data

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr,
                 info_ptr,
                 reinterpret_cast<png_uint_32*>(&w32),
                 reinterpret_cast<png_uint_32*>(&h32),
                 &bit_depth,
                 &color_type,
                 &interlace_type,
                 nullptr,
                 nullptr);

    width          = (int)w32;
    height         = (int)h32;

    int colorModel = DImg::COLORMODELUNKNOWN;
    m_sixteenBit   = (bit_depth == 16);

    switch (color_type)
    {
        case PNG_COLOR_TYPE_RGB:           // RGB
        {
            m_hasAlpha = false;
            colorModel = DImg::RGB;

            break;
        }

        case PNG_COLOR_TYPE_RGB_ALPHA:     // RGBA
        {
            m_hasAlpha = true;
            colorModel = DImg::RGB;

            break;
        }

        case PNG_COLOR_TYPE_GRAY:          // Grayscale
        {
            m_hasAlpha = false;
            colorModel = DImg::GRAYSCALE;

            break;
        }

        case PNG_COLOR_TYPE_GRAY_ALPHA:    // Grayscale + Alpha
        {
            m_hasAlpha = true;
            colorModel = DImg::GRAYSCALE;

            break;
        }

        case PNG_COLOR_TYPE_PALETTE:       // Indexed
        {
            m_hasAlpha = false;
            colorModel = DImg::INDEXED;

            break;
        }
    }

    cleanupData->setColorModel(colorModel);
    cleanupData->setSize(QSize(width, height));

    uchar* data  = nullptr;

    if (m_loadFlags & LoadImageData)
    {
        // TODO: Endianness:
        // You may notice that the code for little and big endian
        // below is now identical. This was found to work by PPC users.
        // If this proves right, all the conditional clauses can be removed.

        if (bit_depth == 16)
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in 16 bits/color/pixel.";

            switch (color_type)
            {
                case PNG_COLOR_TYPE_RGB:           // RGB
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB";
                    png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);

                    break;
                }

                case PNG_COLOR_TYPE_RGB_ALPHA:     // RGBA
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB_ALPHA";

                    break;
                }

                case PNG_COLOR_TYPE_GRAY:          // Grayscale
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY";
                    png_set_gray_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);

                    break;
                }

                case PNG_COLOR_TYPE_GRAY_ALPHA:    // Grayscale + Alpha
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY_ALPHA";
                    png_set_gray_to_rgb(png_ptr);

                    break;
                }

                case PNG_COLOR_TYPE_PALETTE:       // Indexed
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_PALETTE";
                    png_set_palette_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DIMG_LOG_PNG) << "PNG color type unknown.";
                    delete cleanupData;
                    loadingFailed();

                    return false;
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in >=8 bits/color/pixel.";
            png_set_packing(png_ptr);

            switch (color_type)
            {
                case PNG_COLOR_TYPE_RGB:           // RGB
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB";
                    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

                    break;
                }

                case PNG_COLOR_TYPE_RGB_ALPHA:     // RGBA
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB_ALPHA";

                    break;
                }

                case PNG_COLOR_TYPE_GRAY:          // Grayscale
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY";

#if PNG_LIBPNG_VER >= 10400

                    png_set_expand_gray_1_2_4_to_8(png_ptr);

#else

                    png_set_gray_1_2_4_to_8(png_ptr);

#endif

                    png_set_gray_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

                    break;
                }

                case PNG_COLOR_TYPE_GRAY_ALPHA:    // Grayscale + alpha
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY_ALPHA";
                    png_set_gray_to_rgb(png_ptr);

                    break;
                }

                case PNG_COLOR_TYPE_PALETTE:       // Indexed
                {
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_PALETTE";
                    png_set_packing(png_ptr);
                    png_set_palette_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DIMG_LOG_PNG) << "PNG color type unknown." << color_type;
                    delete cleanupData;
                    loadingFailed();

                    return false;
                }
            }
        }

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(png_ptr);
        }

        double file_gamma;

        if (png_get_gAMA(png_ptr, info_ptr, &file_gamma))
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Apply PNG file gamma" << file_gamma;

            png_set_gamma(png_ptr, 2.2, file_gamma);
        }

        png_set_bgr(png_ptr);

        //png_set_swap_alpha(png_ptr);

        if (observer)
        {
            observer->progressInfo(0.1F);
        }

        // -------------------------------------------------------------------
        // Get image data.

        // Call before png_read_update_info and png_start_read_image()
        // for non-interlaced images number_passes will be 1

        int number_passes = png_set_interlace_handling(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        if (m_sixteenBit)
        {
            data = new_failureTolerant(width, height, 8); // 16 bits/color/pixel
        }
        else
        {
            data = new_failureTolerant(width, height, 4); // 8 bits/color/pixel
        }

        cleanupData->setData(data);

        uchar** lines = nullptr;
        (void)lines;    // to prevent cppcheck warnings.
        lines         = reinterpret_cast<unsigned char**>(malloc(height * sizeof(uchar*)));
        cleanupData->setLines(lines);

        if (!data || !lines)
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Cannot allocate memory to load PNG image data.";
            png_read_end(png_ptr, info_ptr);
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);
            delete cleanupData;
            loadingFailed();

            return false;
        }

        for (int i = 0 ; i < height ; ++i)
        {
            if (m_sixteenBit)
            {
                lines[i] = data + ((quint64)i * (quint64)width * 8);
            }
            else
            {
                lines[i] = data + ((quint64)i * (quint64)width * 4);
            }
        }

        // The easy way to read the whole image
        // png_read_image(png_ptr, lines);
        // The other way to read images is row by row. Necessary for observer.
        // Now we need to deal with interlacing.

        for (int pass = 0 ; pass < number_passes ; ++pass)
        {
            int checkPoint = 0;

            for (int y = 0 ; y < height ; ++y)
            {
                if (observer && (y == checkPoint))
                {
                    checkPoint += granularity(observer, height, 0.7F);

                    if (!observer->continueQuery())
                    {
                        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);
                        delete cleanupData;
                        loadingFailed();

                        return false;
                    }

                    // use 10% - 80% for progress while reading rows

                    observer->progressInfo(0.1F + (0.7F * (((float)y) / ((float)height))));
                }

                png_read_rows(png_ptr, lines + y, nullptr, 1);
            }
        }

        cleanupData->freeLines();

        if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        {
            // Swap bytes in 16 bits/color/pixel for DImg

            if (m_sixteenBit)
            {
                uchar ptr[8];   // One pixel to swap

                for (uint p = 0 ; p < (uint)width * height * 8 ; p += 8)
                {
                    memcpy(&ptr[0], &data[p], 8);   // Current pixel

                    data[  p  ] = ptr[1]; // Blue
                    data[p + 1] = ptr[0];
                    data[p + 2] = ptr[3]; // Green
                    data[p + 3] = ptr[2];
                    data[p + 4] = ptr[5]; // Red
                    data[p + 5] = ptr[4];
                    data[p + 6] = ptr[7]; // Alpha
                    data[p + 7] = ptr[6];
                }
            }
        }
    }

    if (observer)
    {
        observer->progressInfo(0.9F);
    }

    // -------------------------------------------------------------------
    // Read image ICC profile

    if (m_loadFlags & LoadICCData)
    {
        png_charp   profile_name;
        iCCP_data   profile_data     = nullptr;
        png_uint_32 profile_size     = 0;
        int         compression_type = 0;

        png_get_iCCP(png_ptr, info_ptr, &profile_name, &compression_type, &profile_data, &profile_size);

        if (profile_data != nullptr)
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
    // Get embedded text data.

    png_text* text_ptr = nullptr;
    int num_comments   = png_get_text(png_ptr, info_ptr, &text_ptr, nullptr);

    /*
    Standard Embedded text includes in PNG :

    Title            Short (one line) title or caption for image
    Author           Name of image's creator
    Description      Description of image (possibly long)
    Copyright        Copyright notice
    Creation Time    Time of original image creation
    Software         Software used to create the image
    Disclaimer       Legal disclaimer
    Warning          Warning of nature of content
    Source           Device used to create the image
    Comment          Miscellaneous comment; conversion from GIF comment

    Extra Raw profiles tag are used by ImageMagick and defines at this Url:
    search.cpan.org/src/EXIFTOOL/Image-ExifTool-5.87/html/TagNames/PNG.html#TextualData
    */

    if (m_loadFlags & LoadICCData)
    {
        for (int i = 0 ; i < num_comments ; ++i)
        {
            // Check if we have a Raw profile embedded using ImageMagick technique.

            if (
                (memcmp(text_ptr[i].key, "Raw profile type exif", 21) != 0) ||
                (memcmp(text_ptr[i].key, "Raw profile type APP1", 21) != 0) ||
                (memcmp(text_ptr[i].key, "Raw profile type iptc", 21) != 0)
               )
            {
                imageSetEmbbededText(QLatin1String(text_ptr[i].key), QLatin1String(text_ptr[i].text));

                qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Reading PNG Embedded text: key=" << text_ptr[i].key
                                              << "size=" << QLatin1String(text_ptr[i].text).size();
            }
        }
    }

    // -------------------------------------------------------------------

    if (m_loadFlags & LoadImageData)
    {
        png_read_end(png_ptr, info_ptr);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);
    cleanupData->takeData();
    delete cleanupData;

    if (observer)
    {
        observer->progressInfo(1.0F);
    }

    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = data;
    imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   bit_depth);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(width, height));

    return true;
}

} // namespace DigikamPNGDImgPlugin
