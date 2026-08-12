/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework - load operations
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpgfloader.h"

// C Ansi includes

extern "C"
{

#ifndef Q_CC_MSVC
#   include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

}

// C++ includes

#include <iostream>
#include <cmath>
#include <cstdio>

// Qt includes

#include <QFile>
#include <QVariant>
#include <qplatformdefs.h>

// Windows includes

#ifdef Q_OS_WIN
#   include <windows.h>
#endif

// Libpgf includes

// Pragma directives to reduce warnings from Libpgf header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#   pragma clang diagnostic ignored "-Wreorder-ctor"
#endif

#include "PGFimage.h"

// Restore warnings
#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "pgfutils.h"
#include "metaengine.h"

namespace Digikam
{

bool DImgPGFLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    m_observer = observer;
    readMetadata(filePath);

#ifdef Q_OS_WIN

    FILE* const file = _wfopen((const wchar_t*)filePath.utf16(), L"rb");

#else

    FILE* const file = fopen(filePath.toUtf8().constData(), "rb");

#endif

    if (!file)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open source file.";

        loadingFailed();

        return false;
    }

    unsigned char header[3];

    if (fread(&header, 3, 1, file) != 1)
    {
        loadingFailed();
        fclose(file);

        return false;
    }

    const unsigned char pgfID[3] = { 0x50, 0x47, 0x46 };

    if (memcmp(&header[0], &pgfID, 3) != 0)
    {
        // not a PGF file

        loadingFailed();
        fclose(file);

        return false;
    }

    fclose(file);

    // -------------------------------------------------------------------
    // Initialize PGF API.

#ifdef Q_OS_WIN

    HANDLE fd = CreateFileW((LPCWSTR)filePath.utf16(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);     // krazy:exclude=null

    if (fd == INVALID_HANDLE_VALUE)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open source file.";
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Last error code:" << GetLastError();
        loadingFailed();

        return false;
    }

#else

    int fd = QT_OPEN(filePath.toUtf8().constData(), O_RDONLY);

    if (fd == -1)
    {
        loadingFailed();

        return false;
    }

#endif

    CPGFFileStream stream(fd);
    CPGFImage      pgf;
    int            colorModel = DImg::COLORMODELUNKNOWN;

    try
    {
        // open pgf image

        pgf.Open(&stream);

        switch (pgf.Mode())
        {
            case ImageModeRGBColor:
            case ImageModeRGB48:
            {
                m_hasAlpha = false;
                colorModel = DImg::RGB;
                break;
            }

            case ImageModeRGBA:
            {
                m_hasAlpha = true;
                colorModel = DImg::RGB;
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Cannot load PGF image: color mode not supported ("
                                                << pgf.Mode() << ")";

#ifdef Q_OS_WIN

                CloseHandle(fd);

#else

                close(fd);

#endif

                loadingFailed();

                return false;
            }
        }

        switch (pgf.Channels())
        {
            case 3:
            case 4:
            {
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Cannot load PGF image: color channels number not supported ("
                                                << pgf.Channels() << ")";

#ifdef Q_OS_WIN

                CloseHandle(fd);

#else

                close(fd);

#endif

                loadingFailed();

                return false;
            }
        }

        int bitDepth = pgf.BPP();

        switch (bitDepth)
        {
            case 24:    // RGB 8 bits.
            case 32:    // RGBA 8 bits.
            {
                m_sixteenBit = false;
                break;
            }

            case 48:    // RGB 16 bits.
            case 64:    // RGBA 16 bits.
            {
                m_sixteenBit = true;
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Cannot load PGF image: color bits depth not supported ("
                                                << bitDepth << ")";

#ifdef Q_OS_WIN

                CloseHandle(fd);

#else

                close(fd);

#endif

                loadingFailed();

                return false;
            }
        }

        if (DIGIKAM_DIMG_LOG_PGF().isDebugEnabled())
        {
            const PGFHeader* const pgfHeader = pgf.GetHeader();
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF width    = " << pgfHeader->width;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF height   = " << pgfHeader->height;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF bbp      = " << pgfHeader->bpp;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF channels = " << pgfHeader->channels;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF quality  = " << pgfHeader->quality;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF mode     = " << pgfHeader->mode;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Has Alpha    = " << m_hasAlpha;
            qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Is 16 bits   = " << m_sixteenBit;
        }

        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properly with libppgf 6.11.24

        pgf.ConfigureDecoder(false);

        int width   = pgf.Width();
        int height  = pgf.Height();
        uchar* data = nullptr;

        QSize originalSize(width, height);

        if (m_loadFlags & LoadImageData)
        {
            // Find out if we do the fast-track loading with reduced size. PGF specific.

            int level          = 0;
            QVariant attribute = imageGetAttribute(QLatin1String("scaledLoadingSize"));

            if (attribute.isValid() && pgf.Levels() > 0)
            {
                int scaledLoadingSize = attribute.toInt();
                int i, w, h;

                for (i = pgf.Levels() - 1 ; i >= 0 ; --i)
                {
                    w = pgf.Width(i);
                    h = pgf.Height(i);

                    if (qMin(w, h) >= scaledLoadingSize)
                    {
                        break;
                    }
                }

                if (i >= 0)
                {
                    width  = w;
                    height = h;
                    level  = i;

                    qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Loading PGF scaled version at level " << i
                                                  << " (" << w << " x " << h << ") for size "
                                                  << scaledLoadingSize;
                }
            }

            if (m_sixteenBit)
            {
                data = new_failureTolerant(width, height, 8); // 16 bits/color/pixel
            }
            else
            {
                data = new_failureTolerant(width, height, 4); // 8 bits/color/pixel
            }

            // Fill all with 255 including alpha channel.

            memset(data, 0xFF, width * height * (m_sixteenBit ? 8 : 4));

            pgf.Read(level, DImgPGFLoader::CallbackForLibPGF, this);
            pgf.GetBitmap(m_sixteenBit ? width * 8 : width * 4,
                          reinterpret_cast<UINT8*>(data),
                          m_sixteenBit ? 64 : 32,
                          nullptr,
                          CallbackForLibPGF, this);

            if (observer)
            {
                observer->progressInfo(1.0F);
            }
        }

        // -------------------------------------------------------------------

#ifdef Q_OS_WIN

        CloseHandle(fd);

#else

        close(fd);

#endif

        // Get ICC color profile.

        if (m_loadFlags & LoadICCData)
        {
            // TODO: Implement proper storage in PGF for color profiles

            checkExifWorkingColorSpace();
        }

        imageWidth()  = width;
        imageHeight() = height;
        imageData()   = data;
        imageSetAttribute(QLatin1String("format"),             QLatin1String("PGF"));
        imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
        imageSetAttribute(QLatin1String("originalBitDepth"),   bitDepth);
        imageSetAttribute(QLatin1String("originalSize"),       originalSize);

        return true;
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Opening and reading PGF image failed (" << err << ")!";

#ifdef Q_OS_WIN

        CloseHandle(fd);

#else

        close(fd);

#endif

        loadingFailed();

        return false;
    }
    catch (std::bad_alloc& e)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Failed to allocate memory for loading" << filePath << e.what();

#ifdef Q_OS_WIN

        CloseHandle(fd);

#else

        close(fd);

#endif

        loadingFailed();

        return false;
    }

    return true;
}

} // namespace Digikam
