/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Antivignetting threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Julien Narboux <julien at narboux dot fr>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original AntiVignettingFilter algorithm copyrighted 2003 by
 * John Walker from 'pnmctrfilt' implementation. See
 * https://www.fourmilab.ch/netpbm/pnmctrfilt for more
 * information.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "antivignettingfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Local includes

#include "dimg.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

AntiVignettingFilter::AntiVignettingFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

AntiVignettingFilter::AntiVignettingFilter(DImg* const orgImage, QObject* const parent,
                                           const AntiVignettingContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("AntiVignettingFilter")),
      m_settings        (settings)
{
    initFilter();
}

AntiVignettingFilter::~AntiVignettingFilter()
{
    cancelFilter();
}

QString AntiVignettingFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Anti-Vignetting Tool"));
}

/**
 * This method is inspired from John Walker 'pnmctrfilt' algorithm code.
 */
void AntiVignettingFilter::filterImage()
{
    int    progress = 0;
    int    col      = 0;
    int    row      = 0;
    int    xd       = 0;
    int    td       = 0;
    int    yd       = 0;
    int    p        = 0;
    int    xsize    = 0;
    int    ysize    = 0;
    int    erad     = 0;
    int    irad     = 0;
    int    xctr     = 0;
    int    yctr     = 0;

    uchar* NewBits               = m_destImage.bits();
    const uchar* data            = m_orgImage.bits();

    unsigned short* NewBits16    = reinterpret_cast<unsigned short*>(m_destImage.bits());
    const unsigned short* data16 = reinterpret_cast<unsigned short*>(m_orgImage.bits());

    int Width                    = m_orgImage.width();
    int Height                   = m_orgImage.height();

    // Determine the shift in pixels from the shift in percentage.

    m_settings.yshift            = m_settings.yshift * Height / 200.0;
    m_settings.xshift            = m_settings.xshift * Width  / 200.0;

    // Determine the outer radius of the filter.  This is the half diagonal
    // measure of the image multiplied by the radius factor.

    xsize    = (Height + 1) / 2;
    ysize    = (Width  + 1) / 2;
    erad     = qRound(hypotenuse(xsize, ysize) * m_settings.outerradius);
    irad     = qRound(hypotenuse(xsize, ysize) * m_settings.outerradius * m_settings.innerradius);
/*
    xsize    = qRound(Width  / 2.0 + fabs(m_settings.xshift));
    ysize    = qRound(Height / 2.0 + fabs(m_settings.yshift));

    diagonal = qRound(hypotenuse(xsize, ysize)) +  1;
*/
    xctr     = qRound(Width  / 2.0 + m_settings.xshift);
    yctr     = qRound(Height / 2.0 + m_settings.yshift);


    for (row = 0 ; runningFlag() && (row < Width) ; ++row)
    {
        yd = abs(xctr - row);

        for (col = 0 ; runningFlag() && (col < Height) ; ++col)
        {
            p  = (col * Width + row) * 4;
            xd = abs(yctr - col);
            td = qRound(hypotenuse(xd, yd));

            if (!m_orgImage.sixteenBit())       // 8 bits image
            {
                NewBits[ p ] = clamp8bits(data[ p ] * real_attenuation(irad, erad, td));
                NewBits[p + 1] = clamp8bits(data[p + 1] * real_attenuation(irad, erad, td));
                NewBits[p + 2] = clamp8bits(data[p + 2] * real_attenuation(irad, erad, td));
                NewBits[p + 3] = data[p + 3];
            }
            else                                // 16 bits image.
            {
                NewBits16[ p ] = clamp16bits(data16[ p ] * real_attenuation(irad, erad, td));
                NewBits16[p + 1] = clamp16bits(data16[p + 1] * real_attenuation(irad, erad, td));
                NewBits16[p + 2] = clamp16bits(data16[p + 2] * real_attenuation(irad, erad, td));
                NewBits16[p + 3] = data16[p + 3];
            }
        }

        // Update the progress bar in dialog.

        progress = (int)(((double)row * 100.0) / Width);

        if ((progress % 5) == 0)
        {
            postProgress(progress);
        }
    }
}

inline double AntiVignettingFilter::hypotenuse(double x, double y)
{
    return (sqrt(x * x + y * y));
}

double AntiVignettingFilter::attenuation(double r1, double r2, double dist_center)
{
    if      (dist_center < r1)
    {
        return 1.0;
    }
    else if (dist_center > r2)
    {
        return 1.0 + m_settings.density;
    }
    else
    {
        return (1.0 + m_settings.density * (pow((dist_center - r1) / (r2 - r1), m_settings.power)));
    }
}

double AntiVignettingFilter::real_attenuation(double r1, double r2, double dist_center)
{
    if (!m_settings.addvignetting)
    {
        return (attenuation(r1, r2, dist_center));
    }
    else
    {
        return (1.0 / attenuation(r1, r2, dist_center));
    }
}

uchar AntiVignettingFilter::clamp8bits(double x)
{
    if      (x < 0)
    {
        return 0;
    }
    else if (x > 255)
    {
        return 255;
    }
    else
    {
        return ((uchar) x);
    }
}

unsigned short  AntiVignettingFilter::clamp16bits(double x)
{
    if      (x < 0)
    {
        return 0;
    }
    else if (x > 65535)
    {
        return 65535;
    }
    else
    {
        return ((unsigned short) x);
    }
}

FilterAction AntiVignettingFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("addvignetting"), m_settings.addvignetting);
    action.addParameter(QLatin1String("density"),       m_settings.density);
    action.addParameter(QLatin1String("innerradius"),   m_settings.innerradius);
    action.addParameter(QLatin1String("outerradius"),   m_settings.outerradius);
    action.addParameter(QLatin1String("power"),         m_settings.power);
    action.addParameter(QLatin1String("xshift"),        m_settings.xshift);
    action.addParameter(QLatin1String("yshift"),        m_settings.yshift);

    return action;
}

void AntiVignettingFilter::readParameters(const Digikam::FilterAction& action)
{
    m_settings.addvignetting = action.parameter(QLatin1String("addvignetting")).toBool();
    m_settings.density       = action.parameter(QLatin1String("density")).toDouble();
    m_settings.innerradius   = action.parameter(QLatin1String("innerradius")).toDouble();
    m_settings.outerradius   = action.parameter(QLatin1String("outerradius")).toDouble();
    m_settings.power         = action.parameter(QLatin1String("power")).toDouble();
    m_settings.xshift        = action.parameter(QLatin1String("xshift")).toDouble();
    m_settings.yshift        = action.parameter(QLatin1String("yshift")).toDouble();
}

} // namespace Digikam

#include "moc_antivignettingfilter.cpp"
