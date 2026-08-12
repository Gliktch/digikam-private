/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Guillaume Castagnino <casta at xwing dot info>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wbfilter.h"

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

class Q_DECL_HIDDEN WBFilter::Private
{
public:

    Private() = default;

    int    BP           = 0;
    int    WP           = 0;

    uint   rgbMax       = 0;

    double curve[65536] = { 0.0 };
    double mr           = 1.0;
    double mg           = 1.0;
    double mb           = 1.0;
};

WBFilter::WBFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

WBFilter::WBFilter(DImg* const orgImage,
                   QObject* const parent,
                   const WBContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("WBFilter")),
      m_settings        (settings),
      d                 (new Private)
{
    initFilter();
}

WBFilter::WBFilter(const WBContainer& settings,
                   DImgThreadedFilter* const master,
                   const DImg& orgImage,
                   const DImg& destImage,
                   int progressBegin,
                   int progressEnd)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, QLatin1String("WBFilter")),
      m_settings        (settings),
      d                 (new Private)
{
    this->filterImage();
}

WBFilter::~WBFilter()
{
    cancelFilter();
    delete d;
}

QString WBFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("White Balance Tool"));
}

void WBFilter::filterImage()
{
    d->WP     = m_orgImage.sixteenBit() ? 65536 : 256;
    d->rgbMax = m_orgImage.sixteenBit() ? 65536 : 256;

    // Set final lut.

    setLUTv();
    setRGBmult(m_settings.temperature, m_settings.green, d->mr, d->mg, d->mb);

    qCDebug(DIGIKAM_DIMG_LOG) << "T(K):" << m_settings.temperature
                              << "=> R:" << d->mr
                              << "   G:" << d->mg
                              << "   B:" << d->mb
                              << "  BP:" << d->BP
                              << "  WP:" << d->WP;

    // Apply White balance adjustments.

    adjustWhiteBalance(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
    m_destImage = m_orgImage;
}

void WBFilter::autoWBAdjustementFromColor(const QColor& tc, double& temperature, double& green)
{
    // Calculate Temperature and Green component from color picked.

    double mr = 0.0;
    double mg = 0.0;
    double mb = 0.0;

    qCDebug(DIGIKAM_DIMG_LOG) << "Sums:  R:" << tc.red() << " G:" << tc.green() << " B:" << tc.blue();

    /**
     * This is a dichotomic search based on Blue and Red layers ratio
     * to find the matching temperature
     * adapted from ufraw (0.12.1) RGB_to_Temperature
     */
    double tmin  = 2000.0;
    double tmax  = 12000.0;
    double mBR   = 0.0;

    if (tc.red() != 0)
    {
        mBR = (double)tc.blue() / (double)tc.red();
    }

    green        = 1.0;

    for (temperature = (tmin + tmax) / 2 ; tmax - tmin > 10 ; temperature = (tmin + tmax) / 2)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Intermediate Temperature (K):" << temperature;
        setRGBmult(temperature, green, mr, mg, mb);

        if ((mb != 0.0) && ((mr / mb) > mBR))
        {
            tmax = temperature;
        }
        else
        {
            tmin = temperature;
        }
    }

    // Calculate the green level to neutralize picture

    if ((mg != 0.0) && (tc.red() != 0))
    {
        green = (mr / mg) / ((double)tc.green() / (double)tc.red());
    }
    else
    {
        green = mr / (double)tc.green();
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Temperature (K):" << temperature;
    qCDebug(DIGIKAM_DIMG_LOG) << "Green component:" << green;
}

void WBFilter::autoExposureAdjustement(const DImg* const img, double& black, double& expo)
{
    // Create an histogram of original image.

    ImageHistogram* const histogram = new ImageHistogram(*img);
    histogram->calculate();

    // Calculate optimal exposition and black level

    int    i;
    double sum, stop;
    uint   rgbMax = img->sixteenBit() ? 65536 : 256;

    // Cutoff at 0.5% of the histogram.

    stop = img->width() * img->height() / 200;

    for (i = rgbMax, sum = 0 ; (i >= 0) && (sum < stop) ; --i)
    {
        sum += histogram->getValue(LuminosityChannel, i);
    }

    expo = -log((double)(i + 1) / rgbMax) / log(2);
    qCDebug(DIGIKAM_DIMG_LOG) << "White level at:" << i;

    for (i = 1, sum = 0 ; (i < (int)rgbMax) && (sum < stop) ; ++i)
    {
        sum += histogram->getValue(LuminosityChannel, i);
    }

    black  = (double)i / rgbMax;
    black /= 2;

    qCDebug(DIGIKAM_DIMG_LOG) << "Black:" << black << "  Exposition:" << expo;

    delete histogram;
}

void WBFilter::setRGBmult(const double& temperature, const double& green, double& mr, double& mg, double& mb)
{
    // Original implementation by Tanner Helland
    // www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/

    double temp = temperature / 100.0;
    double mx;

    if (temp > 120.0)
    {
        temp = 120.0;
    }

    if (temp <= 66.0)
    {
        mr = 1.0;

        mg = temp;
        mg = 99.4708025861 * log(mg) - 161.1195681661;
    }
    else
    {
        mr = temp - 60.0;
        mr = 329.698727466 * pow(mr, -0.1332047592);
        mr = 1.0 / (1.0 / 255.0 * CLAMP(mr, 0.0, 255.0));

        mg = temp - 60.0;
        mg = 288.1221695283 * pow(mg, -0.0755148492);
    }

    mg = 1.0 / 255.0 * CLAMP(mg, 0.0, 255.0);

    // Apply green multiplier

    mg = mg  / green;
    mg = 1.0 / mg;

    if (temp >= 66.0)
    {
        mb = 1.0;
    }
    else
    {
        if (temp <= 19.0)
        {
            mb = 0.0;
        }
        else
        {
            mb = temp - 10.0;
            mb = 138.5177312231 * log(mb) - 305.0447927307;
            mb = 1.0 / (1.0 / 255.0 * CLAMP(mb, 0.0, 255.0));
        }
    }

    // Calculate to an average of 1.0

    mx  = ((mr + mg + mb) / 3.0) - 0.01;
    mr /= mx;
    mg /= mx;
    mb /= mx;
}

void WBFilter::setLUTv()
{
    double b = pow(2, m_settings.expositionMain + m_settings.expositionFine);
    d->BP    = (uint)(d->rgbMax * m_settings.black);
    d->WP    = (uint)(d->rgbMax / b);

    if (d->WP - d->BP < 1)
    {
        d->WP = d->BP + 1;
    }

    d->curve[0] = 0.0;

    // We will try to reproduce the same Gamma effect here than BCG tool.

    double gamma;

    if (m_settings.gamma >= 1.0)
    {
        gamma = 0.335 * (2.0 - m_settings.gamma) + 0.665;
    }
    else
    {
        gamma = 1.8 * (2.0 - m_settings.gamma) - 0.8;
    }

    for (int i = 1 ; i < (int)d->rgbMax ; ++i)
    {
        double x     = (double)(i - d->BP) / (d->WP - d->BP);
        d->curve[i]  = (i < d->BP) ? 0 : (d->rgbMax - 1) * pow((double)x, gamma);
        d->curve[i] *= (1 - m_settings.dark * exp(-x * x / 0.15));
        d->curve[i] /= (double)i;
    }
}

void WBFilter::adjustWhiteBalance(uchar* const data, uint width, uint height, bool sixteenBit)
{
    uint size = width * height;
    uint j;
    int  progress;

    qCDebug(DIGIKAM_DIMG_LOG) << "DImg data:" << data << "Size:"
                              << size << "sixteen bit:" << sixteenBit;

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  red, green, blue;
        uchar* ptr = data;

        for (j = 0 ; runningFlag() && (j < size) ; ++j)
        {
            int idx, rv[3];

            blue   = ptr[0];
            green  = ptr[1];
            red    = ptr[2];

            rv[0]  = (int)(blue  * d->mb);
            rv[1]  = (int)(green * d->mg);
            rv[2]  = (int)(red   * d->mr);
            idx    = qMax(rv[0], rv[1]);
            idx    = qMax(idx, rv[2]);
            idx    = qMin(idx, (int)d->rgbMax - 1);

            ptr[0] = (uchar)pixelColor(rv[0], idx);
            ptr[1] = (uchar)pixelColor(rv[1], idx);
            ptr[2] = (uchar)pixelColor(rv[2], idx);
            ptr   += 4;

            progress = (int)(((double)j * 100.0) / size);

            if ((progress % 5) == 0)
            {
                postProgress(progress);
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short  red, green, blue;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(data);

        for (j = 0 ; runningFlag() && (j < size) ; ++j)
        {
            int idx, rv[3];

            blue   = ptr[0];
            green  = ptr[1];
            red    = ptr[2];

            rv[0]  = (int)(blue  * d->mb);
            rv[1]  = (int)(green * d->mg);
            rv[2]  = (int)(red   * d->mr);
            idx    = qMax(rv[0], rv[1]);
            idx    = qMax(idx, rv[2]);
            idx    = qMin(idx, (int)d->rgbMax - 1);

            ptr[0] = pixelColor(rv[0], idx);
            ptr[1] = pixelColor(rv[1], idx);
            ptr[2] = pixelColor(rv[2], idx);
            ptr   += 4;

            progress = (int)(((double)j * 100.0) / size);

            if ((progress % 5) == 0)
            {
                postProgress(progress);
            }
        }
    }
}

unsigned short WBFilter::pixelColor(int colorMult, int index)
{
    int r = (colorMult > (int)d->rgbMax) ? d->rgbMax : colorMult;
    int c = (index - m_settings.saturation * (index - r)) * d->curve[index];

    return ((unsigned short)CLAMP(c, 0, (int)(d->rgbMax - 1)));
}

FilterAction WBFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    m_settings.writeToFilterAction(action);

    return action;
}

void WBFilter::readParameters(const FilterAction& action)
{
    m_settings = WBContainer::fromFilterAction(action);
}

} // namespace Digikam

#include "moc_wbfilter.cpp"
