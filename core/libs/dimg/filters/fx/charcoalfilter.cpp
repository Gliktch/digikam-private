/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Charcoal threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "charcoalfilter.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QtConcurrentRun>
#include <QMutex>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "blurfilter.h"
#include "stretchfilter.h"
#include "mixerfilter.h"
#include "invertfilter.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN CharcoalFilter::Private
{
public:

    Private() = default;

public:

    const double SQ2PI          = 2.50662827463100024161235523934010416269302368164062;
    const double Epsilon        = 1.0e-12;

    double pencil               = 5.0;
    double smooth               = 10.0;
    int    globalProgress       = 0;

    QMutex lock;
};

CharcoalFilter::CharcoalFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

CharcoalFilter::CharcoalFilter(DImg* const orgImage,
                               QObject* const parent,
                               double pencil,
                               double smooth)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Charcoal")),
      d                 (new Private)
{
    d->pencil = pencil;
    d->smooth = smooth;

    initFilter();
}

CharcoalFilter::~CharcoalFilter()
{
    cancelFilter();
    delete d;
}

QString CharcoalFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Charcoal Effect"));
}

void CharcoalFilter::filterImage()
{
    if (m_orgImage.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "No image data available!";
        return;
    }

    if (d->pencil <= 0.0)
    {
        m_destImage = m_orgImage;
        return;
    }

    // -- Applying Edge effect -----------------------------------------------

    long i          = 0;
    int kernelWidth = getOptimalKernelWidth(d->pencil, d->smooth);

    if ((int)m_orgImage.width() < kernelWidth)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Image is smaller than radius!";
        return;
    }

    QScopedArrayPointer<double> kernel(new double[kernelWidth * kernelWidth] { 0.0 });

    if (kernel.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Unable to allocate memory!";
        return;
    }

    for (i = 0 ; i < (kernelWidth * kernelWidth) ; ++i)
    {
        kernel[i] = (-1.0);
    }

    kernel[i / 2] = kernelWidth * kernelWidth - 1.0;
    convolveImage(kernelWidth, kernel.data());

    // -- Applying Gaussian blur effect ---------------------------------------

    // cppcheck-suppress unusedScopedObject
    BlurFilter(this, m_destImage, m_destImage, 80, 85, (int)(d->smooth / 10.0));

    if (!runningFlag())
    {
        return;
    }

    // -- Applying stretch contrast color effect -------------------------------

    StretchFilter stretch(&m_destImage, &m_destImage);
    stretch.startFilterDirectly();
    m_destImage.putImageData(stretch.getTargetImage().bits());

    postProgress(90);

    if (!runningFlag())
    {
        return;
    }

    // -- Inverting image color -----------------------------------------------

    InvertFilter invert(&m_destImage);
    invert.startFilterDirectly();
    m_destImage.putImageData(invert.getTargetImage().bits());

    postProgress(95);

    if (!runningFlag())
    {
        return;
    }

    // -- Convert to neutral black & white ------------------------------------

    MixerContainer settings;
    settings.bMonochrome    = true;
    settings.blackRedGain   = 0.3;
    settings.blackGreenGain = 0.59;
    settings.blackBlueGain  = 0.11;
    MixerFilter mixer(&m_destImage, nullptr, settings);
    mixer.startFilterDirectly();
    m_destImage.putImageData(mixer.getTargetImage().bits());

    postProgress(100);

    if (!runningFlag())
    {
        return;
    }
}

void CharcoalFilter::convolveImageMultithreaded(uint start, uint stop, double* normal_kernel, double kernelWidth)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    long   mx          = 0;
    long   my          = 0;
    long   sx          = 0;
    long   sy          = 0;
    long   mcx         = 0;
    long   mcy         = 0;
    long   oldProgress = 0;
    long   progress    = 0;
    double red         = 0.0;
    double green       = 0.0;
    double blue        = 0.0;
    double alpha       = 0.0;
    const double* k    = nullptr;

    uint height        = m_destImage.height();
    uint width         = m_destImage.width();
    bool sixteenBit    = m_destImage.sixteenBit();
    uchar* ddata       = m_destImage.bits();
    int ddepth         = m_destImage.bytesDepth();
    uchar* sdata       = m_orgImage.bits();
    int sdepth         = m_orgImage.bytesDepth();
    double maxClamp    = m_destImage.sixteenBit() ? 16777215.0 : 65535.0;

    for (uint y = start ; runningFlag() && (y < stop) ; ++y)
    {
        for (uint x = 0 ; runningFlag() && (x < width) ; ++x)
        {
            k   = normal_kernel;
            red = green = blue = alpha = 0.0;
            sy  = y - (kernelWidth / 2);

            for (mcy = 0 ; runningFlag() && (mcy < kernelWidth) ; ++mcy, ++sy)
            {
                my = (sy < 0) ? 0
                              : (sy > ((int) height - 1)) ? height - 1
                                                          : sy;
                sx = x + (-kernelWidth / 2);

                for (mcx = 0 ; runningFlag() && (mcx < kernelWidth) ; ++mcx, ++sx)
                {
                    mx     = (sx < 0) ? 0
                                      : (sx > ((int) width - 1)) ? width - 1
                                                                 : sx;
                    DColor color(sdata + mx * sdepth + (width * my * sdepth), sixteenBit);
                    red   += (*k) * (color.red()   * 257.0);
                    green += (*k) * (color.green() * 257.0);
                    blue  += (*k) * (color.blue()  * 257.0);
                    alpha += (*k) * (color.alpha() * 257.0);
                    ++k;
                }
            }

            red   = (red < 0.0)   ? 0.0
                                  : (red > maxClamp)   ? maxClamp
                                                       : red + 0.5;

            green = (green < 0.0) ? 0.0
                                  : (green > maxClamp) ? maxClamp
                                                       : green + 0.5;

            blue  = (blue < 0.0)  ? 0.0
                                  : (blue > maxClamp)  ? maxClamp
                                                       : blue + 0.5;

            alpha = (alpha < 0.0) ? 0.0
                                  : (alpha > maxClamp) ? maxClamp
                                                       : alpha + 0.5;

            DColor color((int)(red  / 257UL), (int)(green / 257UL),
                         (int)(blue / 257UL), (int)(alpha / 257UL), sixteenBit);
            color.setPixel((ddata + x * ddepth + (width * y * ddepth)));
        }

        progress = (int)( ( (double)y * (80.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (stop-start));

        if ((progress % 5 == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress        = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

bool CharcoalFilter::convolveImage(const unsigned int order, const double* kernel)
{

#ifndef __clang_analyzer__

    long kernelWidth = order;

    if ((kernelWidth % 2) == 0)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Kernel width must be an odd number!";
        return false;
    }

    long    i;
    double  normalize = 0.0;

    QScopedArrayPointer<double> normal_kernel(new double[kernelWidth * kernelWidth] { 0.0 });

    if (!normal_kernel)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Unable to allocate memory!";

        return false;
    }

    for (i = 0 ; i < (kernelWidth * kernelWidth) ; ++i)
    {
        normalize += kernel[i];
    }

    if (fabs(normalize) <= d->Epsilon)
    {
        normalize = 1.0;
    }

    normalize = 1.0 / normalize;

    for (i = 0 ; i < (kernelWidth * kernelWidth) ; ++i)
    {
        normal_kernel[i] = normalize * kernel[i];
    }

    // --------------------------------------------------------

    QList<int> vals = multithreadedSteps(m_orgImage.height());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                       &CharcoalFilter::convolveImageMultithreaded, this,

#else

                                       this, &CharcoalFilter::convolveImageMultithreaded,

#endif

                                       vals[j],
                                       vals[j+1],
                                       normal_kernel.data(),
                                       kernelWidth
                                      )
        );
    }

    for (QFuture<void> t : std::as_const(tasks))
    {
        t.waitForFinished();
    }

#endif

    return true;
}

int CharcoalFilter::getOptimalKernelWidth(double radius, double sigma)
{
    double normalize, value;
    long   kernelWidth;
    long   u;

    if (radius > 0.0)
    {
        return((int)(2.0 * ceil(radius) + 1.0));
    }

    for (kernelWidth = 5 ; ;)
    {
        normalize = 0.0;

        for (u = (-kernelWidth / 2) ; u <= (kernelWidth / 2) ; ++u)
        {
            normalize += exp(-((double) u * u) / (2.0 * sigma * sigma)) / (d->SQ2PI * sigma);
        }

        u     = kernelWidth / 2;
        value = exp(-((double) u * u) / (2.0 * sigma * sigma)) / (d->SQ2PI * sigma) / normalize;

        if ((long)(65535 * value) <= 0)
        {
            break;
        }

        kernelWidth += 2;
    }

    return ((int)kernelWidth - 2);
}

FilterAction CharcoalFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("pencil"), d->pencil);
    action.addParameter(QLatin1String("smooth"), d->smooth);

    return action;
}

void CharcoalFilter::readParameters(const Digikam::FilterAction& action)
{
    d->pencil = action.parameter(QLatin1String("pencil")).toDouble();
    d->smooth = action.parameter(QLatin1String("smooth")).toDouble();
}

} // namespace Digikam

#include "moc_charcoalfilter.cpp"
