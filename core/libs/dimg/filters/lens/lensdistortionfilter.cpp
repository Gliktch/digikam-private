/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : lens distortion algorithm.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2001-2003 by David Hodson <hodsond@acm.org>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensdistortionfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Local includes

#include "dimg.h"
#include "lensdistortionpixelaccess.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

LensDistortionFilter::LensDistortionFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

LensDistortionFilter::LensDistortionFilter(DImg* const orgImage, QObject* const parent, double main,
                                           double edge, double rescale, double brighten,
                                           int center_x, int center_y)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("LensDistortionFilter")),
      m_centre_x(center_x),
      m_centre_y(center_y),
      m_main    (main),
      m_edge    (edge),
      m_rescale (rescale),
      m_brighten(brighten)
{

    initFilter();
}

LensDistortionFilter::~LensDistortionFilter()
{
    cancelFilter();
}

QString LensDistortionFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Lens Distortion Tool"));
}

void LensDistortionFilter::filterImage()
{
    int    Width      = m_orgImage.width();
    int    Height     = m_orgImage.height();
    int    bytesDepth = m_orgImage.bytesDepth();
    uchar* data       = m_destImage.bits();

    // initial copy

    m_destImage.bitBltImage(&m_orgImage, 0, 0);

    // initialize coefficients

    double normallise_radius_sq   = 4.0    / (Width * Width + Height * Height);
    double center_x               = Width  * (100.0 + m_centre_x) / 200.0;
    double center_y               = Height * (100.0 + m_centre_y) / 200.0;
    double mult_sq                = m_main / 200.0;
    double mult_qd                = m_edge / 200.0;
    double rescale                = pow(2.0, - m_rescale / 100.0);
    double brighten               = - m_brighten / 10.0;
    LensDistortionPixelAccess* pa = new LensDistortionPixelAccess(&m_orgImage);

    /*
     * start at image (i, j), increment by (step, step)
     * output goes to dst, which is w x h x d in size
     * NB: d <= image.bpp
     */

    // We are working on the full image.

    int    dstWidth  = Width;
    int    dstHeight = Height;
    uchar* dst       = reinterpret_cast<unsigned char*>(data);
    int    step      = 1, progress;

    int    iLimit, jLimit;
    double srcX, srcY, mag;

    iLimit = dstWidth  * step;
    jLimit = dstHeight * step;

    for (int dstJ = 0 ; runningFlag() && (dstJ < jLimit) ; dstJ += step)
    {
        for (int dstI = 0 ; runningFlag() && (dstI < iLimit) ; dstI += step)
        {
            // Get source Coordinates.

            double radius_sq;
            double off_x;
            double off_y;
            double radius_mult;

            off_x       = dstI - center_x;
            off_y       = dstJ - center_y;
            radius_sq   = (off_x * off_x) + (off_y * off_y);

            radius_sq  *= normallise_radius_sq;

            radius_mult = radius_sq * mult_sq + radius_sq * radius_sq * mult_qd;
            mag         = radius_mult;
            radius_mult = rescale * (1.0 + radius_mult);

            srcX        = center_x + radius_mult * off_x;
            srcY        = center_y + radius_mult * off_y;

            brighten    = 1.0 + mag * brighten;
            pa->pixelAccessGetCubic(srcX, srcY, brighten, dst);
            dst        += bytesDepth;
        }

        // Update progress bar in dialog.

        progress = (int)(((double)dstJ * 100.0) / jLimit);

        if ((progress % 5) == 0)
        {
            postProgress(progress);
        }
    }

    delete pa;
}

FilterAction LensDistortionFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("brighten"), m_brighten);
    action.addParameter(QLatin1String("centre_x"), m_centre_x);
    action.addParameter(QLatin1String("centre_y"), m_centre_y);
    action.addParameter(QLatin1String("edge"),     m_edge);
    action.addParameter(QLatin1String("main"),     m_main);
    action.addParameter(QLatin1String("rescale"),  m_rescale);

    return action;
}

void LensDistortionFilter::readParameters(const Digikam::FilterAction& action)
{
    m_brighten = action.parameter(QLatin1String("brighten")).toDouble();
    m_centre_x = action.parameter(QLatin1String("centre_x")).toInt();
    m_centre_y = action.parameter(QLatin1String("centre_y")).toInt();
    m_edge     = action.parameter(QLatin1String("edge")).toDouble();
    m_main     = action.parameter(QLatin1String("main")).toDouble();
    m_rescale  = action.parameter(QLatin1String("rescale")).toDouble();
}

} // namespace Digikam

#include "moc_lensdistortionfilter.cpp"
