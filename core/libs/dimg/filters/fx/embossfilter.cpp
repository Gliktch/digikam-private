/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Emboss threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Emboss algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "embossfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QtMath>
#include <QtConcurrentRun>

// Local includes

#include "dimg.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "actionthreadbase.h"

namespace Digikam
{

EmbossFilter::EmbossFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

EmbossFilter::EmbossFilter(DImg* const orgImage, QObject* const parent, int depth)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Emboss")),
      m_depth           (depth)
{
    initFilter();
}

EmbossFilter::~EmbossFilter()
{
    cancelFilter();
}

QString EmbossFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Emboss Effect"));
}

/**
 * Function to apply the EmbossFilter effect
 * This method have been ported from Pieter Z. Voloshyn algorithm code.
 *
 * Theory           => This is an amazing effect. And the theory is very simple to
 *                     understand. You get the difference between the colors and
 *                     increase it. After this, get the gray tone
 */
void EmbossFilter::embossMultithreaded(uint start, uint stop, uint h, double Depth)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    int Width         = m_orgImage.width();
    int Height        = m_orgImage.height();
    bool sixteenBit   = m_orgImage.sixteenBit();
    int bytesDepth    = m_orgImage.bytesDepth();
    uchar* const Bits = m_destImage.bits();

    int    red, green, blue, gray;
    DColor color, colorOther;
    int    offset, offsetOther;

    for (uint w = start ; runningFlag() && (w < stop) ; ++w)
    {
        offset      = getOffset(Width, w, h, bytesDepth);
        offsetOther = getOffset(Width, w + Lim_Max(w, 1, Width), h + Lim_Max(h, 1, Height), bytesDepth);

        color.setColor(Bits + offset, sixteenBit);
        colorOther.setColor(Bits + offsetOther, sixteenBit);

        if (sixteenBit)
        {
            red   = abs((int)((color.red()   - colorOther.red())   * Depth + 32768));
            green = abs((int)((color.green() - colorOther.green()) * Depth + 32768));
            blue  = abs((int)((color.blue()  - colorOther.blue())  * Depth + 32768));

            gray  = CLAMP065535((red + green + blue) / 3);
        }
        else
        {
            red   = abs((int)((color.red()   - colorOther.red())   * Depth + 128));
            green = abs((int)((color.green() - colorOther.green()) * Depth + 128));
            blue  = abs((int)((color.blue()  - colorOther.blue())  * Depth + 128));

            gray  = CLAMP0255((red + green + blue) / 3);
        }

        // Overwrite RGB values to destination. Alpha remains unchanged.

        color.setRed(gray);
        color.setGreen(gray);
        color.setBlue(gray);
        color.setPixel(Bits + offset);
    }
}

void EmbossFilter::filterImage()
{
    // Initial copy

    memcpy(m_destImage.bits(), m_orgImage.bits(), m_destImage.numBytes());

    double Depth = m_depth / 10.0;

    int progress;

    QList<int> vals = multithreadedSteps(m_orgImage.width());

    for (uint h = 0 ; runningFlag() && (h < m_orgImage.height()) ; ++h)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                           &EmbossFilter::embossMultithreaded, this,

#else

                                           this, &EmbossFilter::embossMultithreaded,

#endif

                                           vals[j],
                                           vals[j+1],
                                           h,
                                           Depth
                                          ));
        }

        for (QFuture<void> t : std::as_const(tasks))
        {
            t.waitForFinished();
        }

        progress = (int)(((double)h * 100.0) / m_orgImage.height());

        if ((progress % 5) == 0)
        {
            postProgress(progress);
        }
    }
}

/**
 * Function to limit the max and min values defined by the developer.
 */
int EmbossFilter::Lim_Max(int Now, int Up, int Max)
{
    --Max;

    while (Now > Max - Up)
    {
        --Up;
    }

    return (Up);
}

int EmbossFilter::getOffset(int Width, int X, int Y, int bytesDepth)
{
    return (Y * Width * bytesDepth) + (X * bytesDepth);
}

FilterAction EmbossFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("depth"), m_depth);

    return action;
}

void EmbossFilter::readParameters(const Digikam::FilterAction& action)
{
    m_depth = action.parameter(QLatin1String("depth")).toInt();
}

} // namespace Digikam

#include "moc_embossfilter.cpp"
