/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A QImage loader for DImg framework - load method.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2024 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgqimageloader.h"

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QColorSpace>
#include <QImageReader>

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace DigikamQImageDImgPlugin
{

bool DImgQImageLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath);

    // Loading is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    QImageReader reader(filePath);
    reader.setAutoTransform(false);
    reader.setDecideFormatFromContent(true);

    QByteArray readFormat = reader.format();
    QImage image          = reader.read();

    if (observer)
    {
        observer->progressInfo(0.9F);
    }

    if (image.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Can not load \"" << filePath << "\" using DImg::DImgQImageLoader!";
        qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Error message from loader:" << reader.errorString();
        loadingFailed();

        return false;
    }

    int colorModel    = DImg::COLORMODELUNKNOWN;
    int originalDepth = 0;

    switch (image.format())
    {
        case QImage::Format_Invalid:
        default:
        {
            colorModel    = DImg::COLORMODELUNKNOWN;
            originalDepth = 0;

            break;
        }

        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
        {
            colorModel    = DImg::MONOCHROME;
            originalDepth = 1;

            break;
        }

        case QImage::Format_Indexed8:
        {
            colorModel    = DImg::INDEXED;
            originalDepth = 0;

            break;
        }

        case QImage::Format_RGB32:
        {
            colorModel    = DImg::RGB;
            originalDepth = 8;

            break;
        }

        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
            colorModel    = DImg::RGB;
            originalDepth = 8;

            break;
        }

        case QImage::Format_RGBX64:
        case QImage::Format_RGBA64:
        case QImage::Format_RGBA64_Premultiplied:
        {
            colorModel    = DImg::RGB;
            m_sixteenBit  = true;
            originalDepth = 16;

            break;
        }
    }

    QImage target;
    uint w      = 0;
    uint h      = 0;
    uchar* data = nullptr;

    if (!m_sixteenBit)
    {
        qCDebug(DIGIKAM_DIMG_LOG_QIMAGE) << filePath << "is a 8 bits per color per pixels QImage";

        m_hasAlpha    = (image.hasAlphaChannel() && (readFormat != "psd"));
        target        = image.convertToFormat(QImage::Format_ARGB32);
        w             = target.width();
        h             = target.height();
        data          = new_failureTolerant(w, h, 4);

        if (!data)
        {
            qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Failed to allocate memory for loading" << filePath;
            loadingFailed();

            return false;
        }

        const QRgb* sptr = reinterpret_cast<const QRgb*>(target.constBits());
        uchar*      dptr = data;

        for (uint i = 0 ; i < w * h ; ++i)
        {
            QColor color = QColor::fromRgba(*sptr);
            dptr[0]      = color.blue();
            dptr[1]      = color.green();
            dptr[2]      = color.red();
            dptr[3]      = m_hasAlpha ? color.alpha() : 255;

            dptr   += 4;
            sptr++;
        }
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG_QIMAGE) << filePath << "is a 16 bits per color per pixels QImage";

        m_hasAlpha    = (image.hasAlphaChannel() && (readFormat != "psd"));
        target        = image.convertToFormat(QImage::Format_RGBA64);
        w             = target.width();
        h             = target.height();
        data          = new_failureTolerant(w, h, 8);

        if (!data)
        {
            qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Failed to allocate memory for loading" << filePath;
            loadingFailed();

            return false;
        }

        const QRgba64* sptr = reinterpret_cast<const QRgba64*>(target.constBits());
        ushort*        dptr = reinterpret_cast<ushort*>(data);

        for (uint i = 0 ; i < w * h ; ++i)
        {
            dptr[0] = (*sptr).blue();
            dptr[1] = (*sptr).green();
            dptr[2] = (*sptr).red();
            dptr[3] = m_hasAlpha ? (*sptr).alpha() : 65535;

            dptr   += 4;
            sptr++;
        }
    }

    if (m_loadFlags & LoadICCData)
    {
        QByteArray iccRawProfile(target.colorSpace().iccProfile());

        if (!iccRawProfile.isEmpty())
        {
            imageSetIccProfile(IccProfile(iccRawProfile));
        }
    }

    if (observer)
    {
        observer->progressInfo(1.0F);
    }

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = data;

    QString format(QString::fromLatin1(readFormat).toUpper());

    imageSetAttribute(QLatin1String("format"),             format);
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(w, h));
    imageSetAttribute(QLatin1String("originalBitDepth"),   originalDepth);

    return true;
}

} // namespace DigikamQImageDImgPlugin
