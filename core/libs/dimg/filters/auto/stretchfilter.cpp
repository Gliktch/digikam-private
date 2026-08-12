/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : stretch contrast image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "stretchfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "imagehistogram.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

StretchFilter::StretchFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

StretchFilter::StretchFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("StretchFilter")),
      m_refImage        (*refImage)
{
    initFilter();
}

StretchFilter::~StretchFilter()
{
    cancelFilter();
}

QString StretchFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Stretch Contrast"));
}

void StretchFilter::filterImage()
{
    if (m_refImage.isNull())
    {
        m_refImage = m_orgImage;
    }

    stretchContrastImage();
    m_destImage = m_orgImage;
}

/**
 * Performs histogram normalization of the image. The algorithm normalizes
 * the pixel values from an image for to span the full range
 * of color values. This is a contrast enhancement technique.
 */
void StretchFilter::stretchContrastImage()
{
    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Ref. image and Org. has different bits depth";
        return;
    }

    struct double_packet high, low, intensity;
    uint                 threshold_intensity;
    uint                 number_pixels;
    int                  progress;

    // Create an histogram of the reference image.

    QScopedPointer<ImageHistogram> histogram(new ImageHistogram(m_refImage));

    if (histogram.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Unable to allocate memory!";
        return;
    }

    histogram->calculate();

    // Memory allocation.

    QScopedArrayPointer<int_packet> normalize_map(new int_packet[histogram->getHistogramSegments()]);

    if (normalize_map.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Unable to allocate memory!";
        return;
    }

    // Find the histogram boundaries by locating the 0.1 percent levels.

    number_pixels       = (uint)m_refImage.width() * (uint)m_refImage.height();
    threshold_intensity = number_pixels / 1000;

    // Red.

    intensity = double_packet();

    for (high.red = histogram->getMaxSegmentIndex() ; high.red != 0 ; --high.red)
    {
        intensity.red += histogram->getValue(RedChannel, (int)high.red);

        if (intensity.red > threshold_intensity)
        {
            break;
        }
    }

    if (low.red == high.red)
    {
        threshold_intensity = 0;
        intensity           = double_packet();

        for (low.red = 0 ; low.red < histogram->getMaxSegmentIndex() ; ++low.red)
        {
            intensity.red += histogram->getValue(RedChannel, (int)low.red);

            if (intensity.red > threshold_intensity)
            {
                break;
            }
        }

        intensity = double_packet();

        for (high.red = histogram->getMaxSegmentIndex() ; high.red != 0 ; --high.red)
        {
            intensity.red += histogram->getValue(RedChannel, (int)high.red);

            if (intensity.red > threshold_intensity)
            {
                break;
            }
        }
    }

    // Green.

    intensity = double_packet();

    for (high.green = histogram->getMaxSegmentIndex() ; high.green != 0 ; --high.green)
    {
        intensity.green += histogram->getValue(GreenChannel, (int)high.green);

        if (intensity.green > threshold_intensity)
        {
            break;
        }
    }

    if (low.green == high.green)
    {
        threshold_intensity = 0;
        intensity           = double_packet();

        for (low.green = 0 ; low.green < histogram->getMaxSegmentIndex() ; ++low.green)
        {
            intensity.green += histogram->getValue(GreenChannel, (int)low.green);

            if (intensity.green > threshold_intensity)
            {
                break;
            }
        }

        intensity = double_packet();

        for (high.green = histogram->getMaxSegmentIndex() ; high.green != 0 ; --high.green)
        {
            intensity.green += histogram->getValue(GreenChannel, (int)high.green);

            if (intensity.green > threshold_intensity)
            {
                break;
            }
        }
    }

    // Blue.

    intensity = double_packet();

    for (high.blue = histogram->getMaxSegmentIndex() ; high.blue != 0 ; --high.blue)
    {
        intensity.blue += histogram->getValue(BlueChannel, (int)high.blue);

        if (intensity.blue > threshold_intensity)
        {
            break;
        }
    }

    if (low.blue == high.blue)
    {
        threshold_intensity = 0;
        intensity           = double_packet();

        for (low.blue = 0 ; low.blue < histogram->getMaxSegmentIndex() ; ++low.blue)
        {
            intensity.blue += histogram->getValue(BlueChannel, (int)low.blue);

            if (intensity.blue > threshold_intensity)
            {
                break;
            }
        }

        intensity = double_packet();

        for (high.blue = histogram->getMaxSegmentIndex() ; high.blue != 0 ; --high.blue)
        {
            intensity.blue += histogram->getValue(BlueChannel, (int)high.blue);

            if (intensity.blue > threshold_intensity)
            {
                break;
            }
        }
    }

    // Alpha.

    intensity = double_packet();

    for (high.alpha = histogram->getMaxSegmentIndex() ; high.alpha != 0 ; --high.alpha)
    {
        intensity.alpha += histogram->getValue(AlphaChannel, (int)high.alpha);

        if (intensity.alpha > threshold_intensity)
        {
            break;
        }
    }

    if (low.alpha == high.alpha)
    {
        threshold_intensity = 0;
        intensity           = double_packet();

        for (low.alpha = 0 ; low.alpha < histogram->getMaxSegmentIndex() ; ++low.alpha)
        {
            intensity.alpha += histogram->getValue(AlphaChannel, (int)low.alpha);

            if (intensity.alpha > threshold_intensity)
            {
                break;
            }
        }

        intensity = double_packet();

        for (high.alpha = histogram->getMaxSegmentIndex() ; high.alpha != 0 ; --high.alpha)
        {
            intensity.alpha += histogram->getValue(AlphaChannel, (int)high.alpha);

            if (intensity.alpha > threshold_intensity)
            {
                break;
            }
        }
    }

    // Stretch the histogram to create the normalized image mapping.
    // TODO magic number 256

    for (int i = 0 ; runningFlag() && (i <= histogram->getMaxSegmentIndex()) ; ++i)
    {
        if      (i < (int) low.red)
        {
            normalize_map[i].red = 0;
        }
        else if (i > (int) high.red)
        {
            normalize_map[i].red = (256 * histogram->getHistogramSegments() - 1);
        }
        else if (low.red != high.red)
        {
            normalize_map[i].red = (int)(((256 * histogram->getHistogramSegments() - 1) * (i - low.red)) / (high.red - low.red));
        }

        if      (i < (int) low.green)
        {
            normalize_map[i].green = 0;
        }
        else if (i > (int) high.green)
        {
            normalize_map[i].green = (256 * histogram->getHistogramSegments() - 1);
        }
        else if (low.green != high.green)
        {
            normalize_map[i].green = (int)(((256 * histogram->getHistogramSegments() - 1) * (i - low.green)) / (high.green - low.green));
        }

        if      (i < (int) low.blue)
        {
            normalize_map[i].blue = 0;
        }
        else if (i > (int) high.blue)
        {
            normalize_map[i].blue = (256 * histogram->getHistogramSegments() - 1);
        }
        else if (low.blue != high.blue)
        {
            normalize_map[i].blue = (int)(((256 * histogram->getHistogramSegments() - 1) * (i - low.blue)) / (high.blue - low.blue));
        }

        if      (i < (int) low.alpha)
        {
            normalize_map[i].alpha = 0;
        }
        else if (i > (int) high.alpha)
        {
            normalize_map[i].alpha = (256 * histogram->getHistogramSegments() - 1);
        }
        else if (low.alpha != high.alpha)
        {
            normalize_map[i].alpha = (int)(((256 * histogram->getHistogramSegments() - 1) * (i - low.alpha)) / (high.alpha - low.alpha));
        }
    }

    // Apply result to image.

    uchar* data     = m_orgImage.bits();
    uint w          = m_orgImage.width();
    uint h          = m_orgImage.height();
    bool sixteenBit = m_orgImage.sixteenBit();
    uint size       = w * h;

    // TODO magic number 257

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  red, green, blue, alpha;
        uchar* ptr = data;

        for (uint i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (low.red != high.red)
            {
                red = (normalize_map[red].red) / 257;
            }

            if (low.green != high.green)
            {
                green = (normalize_map[green].green) / 257;
            }

            if (low.blue != high.blue)
            {
                blue = (normalize_map[blue].blue) / 257;
            }

            if (low.alpha != high.alpha)
            {
                alpha = (normalize_map[alpha].alpha) / 257;
            }

            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr   += 4;

            progress = (int)(((double)i * 100.0) / size);

            if ((progress % 5) == 0)
            {
                postProgress(progress);
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short  red, green, blue, alpha;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(data);

        for (uint i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (low.red != high.red)
            {
                red = (normalize_map[red].red) / 257;
            }

            if (low.green != high.green)
            {
                green = (normalize_map[green].green) / 257;
            }

            if (low.blue != high.blue)
            {
                blue = (normalize_map[blue].blue) / 257;
            }

            if (low.alpha != high.alpha)
            {
                alpha = (normalize_map[alpha].alpha) / 257;
            }

            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr   += 4;

            progress = (int)(((double)i * 100.0) / size);

            if ((progress % 5) == 0)
            {
                postProgress(progress);
            }
        }
    }
}

FilterAction StretchFilter::filterAction()
{
    return DefaultFilterAction<StretchFilter>();
}

void StretchFilter::readParameters(const FilterAction& /*action*/)
{
    return;
}

} // namespace Digikam

#include "moc_stretchfilter.cpp"
