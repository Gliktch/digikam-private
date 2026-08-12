/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework - save operations
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

bool DImgPGFLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    m_observer = observer;

#ifdef Q_OS_WIN

    HANDLE fd = CreateFileW((LPCWSTR)filePath.utf16(), GENERIC_READ | GENERIC_WRITE, 0,
                            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);             // krazy:exclude=null

    if (fd == INVALID_HANDLE_VALUE)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open destination file.";
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Last error code:" << GetLastError();

        return false;
    }

#else

    int fd = QT_OPEN(filePath.toUtf8().constData(),
                     O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Could not open destination file.";

        return false;
    }

#endif

    try
    {
        QVariant losslessAttr = imageGetAttribute(QLatin1String("lossless"));
        bool lossless         = losslessAttr.isValid() ? losslessAttr.toBool() : false;

        // NOTE: if quality = 0 -> lossless compression.

        int quality           = 0;

        if (!lossless)
        {
            QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
            quality              = qualityAttr.isValid() ? qualityAttr.toInt() : 3;
        }

        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF quality: " << quality;

        CPGFFileStream stream(fd);
        CPGFImage      pgf;
        PGFHeader      header;
        header.width   = imageWidth();
        header.height  = imageHeight();
        header.quality = quality;

        if (imageHasAlpha())
        {
            if (imageSixteenBit())
            {
                // NOTE : there is no PGF color mode in 16 bits with alpha.

                header.channels = 3;
                header.bpp      = 48;
                header.mode     = ImageModeRGB48;
            }
            else
            {
                header.channels = 4;
                header.bpp      = 32;
                header.mode     = ImageModeRGBA;
            }
        }
        else
        {
            if (imageSixteenBit())
            {
                header.channels = 3;
                header.bpp      = 48;
                header.mode     = ImageModeRGB48;
            }
            else
            {
                header.channels = 3;
                header.bpp      = 24;
                header.mode     = ImageModeRGBColor;
            }
        }

#ifdef PGFCodecVersionID

#   if PGFCodecVersionID < 0x061142

        header.background.rgbtBlue  = 0;
        header.background.rgbtGreen = 0;
        header.background.rgbtRed   = 0;

#   endif

#endif

        pgf.SetHeader(header);

        // NOTE: see bug #273765 : Loading PGF thumbs with OpenMP support through a separated thread do not work properly with libppgf 6.11.24

        pgf.ConfigureEncoder(false);

        pgf.ImportBitmap(4 * imageWidth() * (imageSixteenBit() ? 2 : 1),
                         reinterpret_cast<UINT8*>(imageData()),
                         imageBitsDepth() * 4,
                         nullptr,
                         DImgPGFLoader::CallbackForLibPGF, this);

        UINT32 nWrittenBytes = 0;

#ifdef PGFCodecVersionID

#   if PGFCodecVersionID >= 0x061124

        pgf.Write(&stream, &nWrittenBytes, CallbackForLibPGF, this);

#   endif

#else

        pgf.Write(&stream, 0, CallbackForLibPGF, &nWrittenBytes, this);

#endif

        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF width     = " << header.width;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF height    = " << header.height;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF bbp       = " << header.bpp;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF channels  = " << header.channels;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF quality   = " << header.quality;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "PGF mode      = " << header.mode;
        qCDebug(DIGIKAM_DIMG_LOG_PGF) << "Bytes Written = " << nWrittenBytes;

#ifdef Q_OS_WIN

        CloseHandle(fd);

#else

        close(fd);

#endif

        // TODO: Store ICC profile in an appropriate place in the image

        storeColorProfileInMetadata();

        if (observer)
        {
            observer->progressInfo(1.0F);
        }

        imageSetAttribute(QLatin1String("savedFormat"), QLatin1String("PGF"));
        saveMetadata(filePath);

        return true;
    }
    catch (IOException& e)
    {
        int err = e.error;

        if (err >= AppError)
        {
            err -= AppError;
        }

        qCWarning(DIGIKAM_DIMG_LOG_PGF) << "Error: Opening and saving PGF image failed (" << err << ")!";

#ifdef Q_OS_WIN

        CloseHandle(fd);

#else

        close(fd);

#endif

        return false;
    }

    return true;
}

} // namespace Digikam
