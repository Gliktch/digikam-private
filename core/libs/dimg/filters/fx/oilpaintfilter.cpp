/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Oil Painting threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original OilPaint algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "oilpaintfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QtConcurrentRun>
#include <QMutex>

// Local includes

#include "dimg.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN OilPaintFilter::Private
{
public:

    Private() = default;

public:

    int    brushSize        = 1;
    int    smoothness       = 30;

    int    globalProgress   = 0;

    QMutex lock;
};

OilPaintFilter::OilPaintFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

OilPaintFilter::OilPaintFilter(DImg* const orgImage, QObject* const parent, int brushSize, int smoothness)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("OilPaintFilter")),
      d                 (new Private)
{
    d->brushSize  = brushSize;
    d->smoothness = smoothness;

    initFilter();
}

OilPaintFilter::~OilPaintFilter()
{
    cancelFilter();
    delete d;
}

QString OilPaintFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Oil Painter Effect"));
}

/**
 * Function to apply the OilPaintFilter effect.
 * This method have been ported from Pieter Z. Voloshyn algorithm code.
 *
 * Theory: Using MostFrequentColor function we take the main color in
 *         a matrix and simply write at the original position.
 */
void OilPaintFilter::oilPaintImageMultithreaded(uint start, uint stop)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    QScopedArrayPointer<uchar> intensityCount(new uchar[d->smoothness + 1]);
    QScopedArrayPointer<uint>  averageColorR(new  uint[d->smoothness + 1]);
    QScopedArrayPointer<uint>  averageColorG(new  uint[d->smoothness + 1]);
    QScopedArrayPointer<uint>  averageColorB(new  uint[d->smoothness + 1]);

    memset(intensityCount.data(), 0, sizeof(uchar)*(d->smoothness + 1));
    memset(averageColorR.data(),  0, sizeof(uint)*(d->smoothness + 1));
    memset(averageColorG.data(),  0, sizeof(uint)*(d->smoothness + 1));
    memset(averageColorB.data(),  0, sizeof(uint)*(d->smoothness + 1));

    int    oldProgress=0, progress=0;
    DColor mostFrequentColor;

    mostFrequentColor.setSixteenBit(m_orgImage.sixteenBit());
    uchar* dest    = m_destImage.bits();
    uchar* dptr    = nullptr;

    for (uint h2 = start ; runningFlag() && (h2 < stop) ; ++h2)
    {
        for (uint w2 = 0 ; runningFlag() && (w2 < m_orgImage.width()) ; ++w2)
        {
            mostFrequentColor = MostFrequentColor(m_orgImage, w2, h2, d->brushSize, d->smoothness,
                                                  intensityCount.data(), averageColorR.data(), averageColorG.data(), averageColorB.data());
            dptr              = dest + w2 * m_orgImage.bytesDepth() + (m_orgImage.width() * h2 * m_orgImage.bytesDepth());
            mostFrequentColor.setPixel(dptr);
        }

        progress = (int)( ( (double)h2 * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (stop-start));

        if (((progress % 5) == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress       = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

void OilPaintFilter::filterImage()
{
    QList<int> vals = multithreadedSteps(m_orgImage.height());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                       &OilPaintFilter::oilPaintImageMultithreaded, this,

#else

                                       this, &OilPaintFilter::oilPaintImageMultithreaded,

#endif

                                       vals[j],
                                       vals[j+1]
                                      )
        );
    }

    for (QFuture<void> t : std::as_const(tasks))
    {
        t.waitForFinished();
    }
}

/**
 * Function to determine the most frequent color in a matrix
 *
 * Bits             => Bits array
 * Width            => Image width
 * Height           => Image height
 * X                => Position horizontal
 * Y                => Position vertical
 * Radius           => Is the radius of the matrix to be analyzed
 * Intensity        => Intensity to calculate
 *
 * Theory           => This function creates a matrix with the analyzed pixel in
 *                     the center of this matrix and find the most frequently color
 */
DColor OilPaintFilter::MostFrequentColor(const DImg& src, int X, int Y, int Radius, int Intensity,
                                         uchar* intensityCount, uint* averageColorR,
                                         uint* averageColorG, uint* averageColorB)
{
    int  i, w, h, I, Width, Height;
    uint red, green, blue;

    uchar* dest     = src.bits();
    int bytesDepth  = src.bytesDepth();
    uchar* sptr     = nullptr;
    bool sixteenBit = src.sixteenBit();

    DColor mostFrequentColor;

    double Scale = Intensity / (sixteenBit ? 65535.0 : 255.0);
    Width        = (int)src.width();
    Height       = (int)src.height();

    // Erase the array

    memset(intensityCount, 0, (Intensity + 1) * sizeof(uchar));

    for (w = X - Radius ; w <= X + Radius ; ++w)
    {
        for (h = Y - Radius ; h <= Y + Radius ; ++h)
        {
            // This condition helps to identify when a point doesn't exist

            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
            {
                sptr          = dest + w * bytesDepth + (Width * h * bytesDepth);
                DColor color(sptr, sixteenBit);
                red           = (uint)color.red();
                green         = (uint)color.green();
                blue          = (uint)color.blue();

                I = lround(GetIntensity(red, green, blue) * Scale);
                intensityCount[I]++;

                if (intensityCount[I] == 1)
                {
                    averageColorR[I] = red;
                    averageColorG[I] = green;
                    averageColorB[I] = blue;
                }
                else
                {
                    averageColorR[I] += red;
                    averageColorG[I] += green;
                    averageColorB[I] += blue;
                }
            }
        }
    }

    I               = 0;
    int MaxInstance = 1;

    for (i = 0 ; i <= Intensity ; ++i)
    {
        if (intensityCount[i] > MaxInstance)
        {
            I = i;
            MaxInstance = intensityCount[i];
        }
    }

    // get Alpha channel value from original (unchanged)

    mostFrequentColor = src.getPixelColor(X, Y);

    // Overwrite RGB values to destination.

    mostFrequentColor.setRed(averageColorR[I]   / MaxInstance);
    mostFrequentColor.setGreen(averageColorG[I] / MaxInstance);
    mostFrequentColor.setBlue(averageColorB[I]  / MaxInstance);

    return mostFrequentColor;
}

/**
 * Function to calculate the color intensity and return the luminance (Y)
 * component of YIQ color model.
 */
double OilPaintFilter::GetIntensity(uint Red, uint Green, uint Blue)
{
    return (Red * 0.3 + Green * 0.59 + Blue * 0.11);
}

FilterAction OilPaintFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("brushSize"),  d->brushSize);
    action.addParameter(QLatin1String("smoothness"), d->smoothness);

    return action;
}

void OilPaintFilter::readParameters(const Digikam::FilterAction& action)
{
    d->brushSize  = action.parameter(QLatin1String("brushSize")).toInt();
    d->smoothness = action.parameter(QLatin1String("smoothness")).toInt();
}

} // namespace Digikam

#include "moc_oilpaintfilter.cpp"
