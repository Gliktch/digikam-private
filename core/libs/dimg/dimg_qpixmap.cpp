/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               QPixmap accessors.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimg_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN PixmapPaintEngineDetector
{
public:

    PixmapPaintEngineDetector()
        : m_isRaster(detectRasterFromPixmap())
    {
    }

    bool isRaster() const
    {
        return m_isRaster;
    }

private:

    static bool detectRasterFromPixmap()
    {
        QPixmap pix(1, 1);
        QPainter p(&pix);

        return (p.paintEngine() && (p.paintEngine()->type() == QPaintEngine::Raster));
    }

    const bool m_isRaster;
};

Q_GLOBAL_STATIC(PixmapPaintEngineDetector, pixmapPaintEngineDetector)

// --------------------------------------------------------------------------------------

QPixmap DImg::convertToPixmap() const
{
    if (isNull())
    {
        return QPixmap();
    }

    if (sixteenBit())
    {
        // make faster...

        return QPixmap::fromImage(copyQImage(0, 0, width(), height()));
    }

    if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
    {
        QImage img(width(), height(), hasAlpha() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

        uchar* sptr = bits();
        uint*  dptr = reinterpret_cast<uint*>(img.bits());
        uint dim    = width() * height();

        for (uint i = 0 ; i < dim ; ++i)
        {
            *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], sptr[3]);
            sptr   += 4;
        }

        // alpha channel is auto-detected during QImage->QPixmap conversion

        return QPixmap::fromImage(img);
    }
    else
    {
        // This is a temporary image operating on the DImg buffer

        QImage img(bits(), width(), height(), hasAlpha() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

        // For paint engines which base the QPixmap internally on a QImage, we must use a persistent QImage

        if (pixmapPaintEngineDetector->isRaster())
        {
            img = img.copy();
        }

        // alpha channel is auto-detected during QImage->QPixmap conversion

        return QPixmap::fromImage(img);
    }
}

QPixmap DImg::convertToPixmap(IccTransform& monitorICCtrans) const
{
    if (isNull())
    {
        return QPixmap();
    }

    if (monitorICCtrans.outputProfile().isNull())
    {
        return convertToPixmap();
    }

    DImg img = copy();
    monitorICCtrans.apply(img);

    return (img.convertToPixmap());
}

QImage DImg::pureColorMask(ExposureSettingsContainer* const expoSettings) const
{
    if (isNull() || (!expoSettings->underExposureIndicator && !expoSettings->overExposureIndicator))
    {
        return QImage();
    }

    QImage img(size(), QImage::Format_ARGB32);
    img.fill(0x00000000);      // Full transparent.

    /**
     * @note Qt4 and later do not provide anymore QImage::setAlphaChannel() because
     * alpha channel is auto-detected during QImage->QPixmap conversion.
     */
    uchar* ibits = img.bits();

    int    max  = lround(sixteenBit() ? 65535.0 - (65535.0 * expoSettings->overExposurePercent  / 100.0)
                                      : 255.0   - (255.0   * expoSettings->overExposurePercent  / 100.0));
    int    min  = lround(sixteenBit() ? 0.0     + (65535.0 * expoSettings->underExposurePercent / 100.0)
                                      : 0.0     + (255.0   * expoSettings->underExposurePercent / 100.0));

    // --------------------------------------------------------

    // caching

    int u_red   = expoSettings->underExposureColor.red();
    int u_green = expoSettings->underExposureColor.green();
    int u_blue  = expoSettings->underExposureColor.blue();

    int o_red   = expoSettings->overExposureColor.red();
    int o_green = expoSettings->overExposureColor.green();
    int o_blue  = expoSettings->overExposureColor.blue();

    bool under  = expoSettings->underExposureIndicator;
    bool over   = expoSettings->overExposureIndicator;
    bool pure   = expoSettings->exposureIndicatorMode;

    // --------------------------------------------------------

    uint   dim    = m_priv->width * m_priv->height;
    uchar* dptr   = ibits;
    int    s_blue = 0, s_green = 0, s_red = 0;
    bool   match  = false;

    if (sixteenBit())
    {
        const unsigned short* sptr = reinterpret_cast<unsigned short*>(m_priv->data);

        for (uint i = 0 ; i < dim ; ++i)
        {
            s_blue  = *sptr++;
            s_green = *sptr++;
            s_red   = *sptr++;
            sptr++;
            match = pure ? (s_red <= min) && (s_green <= min) && (s_blue <= min)
                         : (s_red <= min) || (s_green <= min) || (s_blue <= min);

            if (under && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = u_red;
                    dptr[2] = u_green;
                    dptr[3] = u_blue;
                }
                else
                {
                    dptr[0] = u_blue;
                    dptr[1] = u_green;
                    dptr[2] = u_red;
                    dptr[3] = 0xFF;
                }
            }

            match = pure ? (s_red >= max) && (s_green >= max) && (s_blue >= max)
                         : (s_red >= max) || (s_green >= max) || (s_blue >= max);

            if (over && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = o_red;
                    dptr[2] = o_green;
                    dptr[3] = o_blue;
                }
                else
                {
                    dptr[0] = o_blue;
                    dptr[1] = o_green;
                    dptr[2] = o_red;
                    dptr[3] = 0xFF;
                }
            }

            dptr += 4;
        }
    }
    else
    {
        const uchar* sptr = m_priv->data;

        for (uint i = 0 ; i < dim ; ++i)
        {
            s_blue  = *sptr++;
            s_green = *sptr++;
            s_red   = *sptr++;
            sptr++;
            match = pure ? (s_red <= min) && (s_green <= min) && (s_blue <= min)
                         : (s_red <= min) || (s_green <= min) || (s_blue <= min);

            if (under && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = u_red;
                    dptr[2] = u_green;
                    dptr[3] = u_blue;
                }
                else
                {
                    dptr[0] = u_blue;
                    dptr[1] = u_green;
                    dptr[2] = u_red;
                    dptr[3] = 0xFF;
                }
            }

            match = pure ? (s_red >= max) && (s_green >= max) && (s_blue >= max)
                         : (s_red >= max) || (s_green >= max) || (s_blue >= max);

            if (over && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = o_red;
                    dptr[2] = o_green;
                    dptr[3] = o_blue;
                }
                else
                {
                    dptr[0] = o_blue;
                    dptr[1] = o_green;
                    dptr[2] = o_red;
                    dptr[3] = 0xFF;
                }
            }

            dptr += 4;
        }
    }

    return img;
}

} // namespace Digikam
