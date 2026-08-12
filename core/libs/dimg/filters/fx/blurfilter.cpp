/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blurfilter.h"

// Qt includes

#include <QtConcurrentRun>
#include <QtMath>
#include <QMutex>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN BlurFilter::Private
{
public:

    Private() = default;

    int    radius         = 3;
    int    globalProgress = 0;

    QMutex lock;
};

BlurFilter::BlurFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

BlurFilter::BlurFilter(DImg* const orgImage, QObject* const parent, int radius)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("GaussianBlur")),
      d                 (new Private)
{
    d->radius = radius;
    initFilter();
}

BlurFilter::BlurFilter(DImgThreadedFilter* const parentFilter,
                       const DImg& orgImage,
                       const DImg& destImage,
                       int progressBegin,
                       int progressEnd,
                       int radius)
    : DImgThreadedFilter(parentFilter,
                         orgImage,
                         destImage,
                         progressBegin,
                         progressEnd,
                         parentFilter->filterName() + QLatin1String(": GaussianBlur")),
      d(new Private)
{
    d->radius = radius;
    this->filterImage();
}

BlurFilter::~BlurFilter()
{
    cancelFilter();

    delete d;
}

QString BlurFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Blur Filter"));
}

void BlurFilter::blurMultithreaded(uint start, uint stop)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    bool sixteenBit  = m_orgImage.sixteenBit();
    int  height      = m_orgImage.height();
    int  width       = m_orgImage.width();
    int  radius      = d->radius;
    int  oldProgress = 0;
    int  progress    = 0;
    uint a, r, g, b;
    int  mx;
    int  my;
    int  mw;
    int  mh;
    int  mt;
    int* as = new int[width];
    int* rs = new int[width];
    int* gs = new int[width];
    int* bs = new int[width];

    for (uint y = start ; runningFlag() && (y < stop) ; ++y)
    {
        my = y - radius;
        mh = (radius << 1) + 1;

        // cppcheck-suppress knownConditionTrueFalse
        if (my < 0)
        {
            mh += my;
            my  = 0;
        }

        if ((my + mh) > height)
        {
            mh = height - my;
        }

        uchar* pDst8           = m_destImage.scanLine(y);
        unsigned short* pDst16 = reinterpret_cast<unsigned short*>(m_destImage.scanLine(y));

        memset(as, 0, width * sizeof(int));
        memset(rs, 0, width * sizeof(int));
        memset(gs, 0, width * sizeof(int));
        memset(bs, 0, width * sizeof(int));

        for (int yy = 0 ; yy < mh ; ++yy)
        {
            uchar* pSrc8           = m_orgImage.scanLine(yy + my);
            unsigned short* pSrc16 = reinterpret_cast<unsigned short*>(m_orgImage.scanLine(yy + my));

            for (int x = 0 ; x < width ; ++x)
            {
                if (sixteenBit)
                {
                    bs[x]  += pSrc16[0];
                    gs[x]  += pSrc16[1];
                    rs[x]  += pSrc16[2];
                    as[x]  += pSrc16[3];
                    pSrc16 += 4;
                }
                else
                {
                    bs[x] += pSrc8[0];
                    gs[x] += pSrc8[1];
                    rs[x] += pSrc8[2];
                    as[x] += pSrc8[3];
                    pSrc8 += 4;
                }
            }
        }

        if (width > ((radius << 1) + 1))
        {
            for (int x = 0 ; x < width ; ++x)
            {
                a  = 0;
                r  = 0;
                g  = 0;
                b  = 0;
                mx = x - radius;
                mw = (radius << 1) + 1;

                if (mx < 0)
                {
                    mw += mx;
                    mx  = 0;
                }

                if ((mx + mw) > width)
                {
                    mw = width - mx;
                }

                mt = mw * mh;

                for (int xx = mx ; xx < (mw + mx) ; ++xx)
                {
                    a += as[xx];
                    r += rs[xx];
                    g += gs[xx];
                    b += bs[xx];
                }

                if (mt != 0)
                {
                    a = a / mt;
                    r = r / mt;
                    g = g / mt;
                    b = b / mt;
                }

                if (sixteenBit)
                {
                    pDst16[0] = b;
                    pDst16[1] = g;
                    pDst16[2] = r;
                    pDst16[3] = a;
                    pDst16   += 4;
                }
                else
                {
                    pDst8[0] = b;
                    pDst8[1] = g;
                    pDst8[2] = r;
                    pDst8[3] = a;
                    pDst8   += 4;
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Radius too small...";
        }

        progress = (int)(((double)y * (100.0 / QThreadPool::globalInstance()->maxThreadCount())) / (stop-start));

        if (((progress % 5) == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress        = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }

    delete [] as;
    delete [] rs;
    delete [] gs;
    delete [] bs;
}

void BlurFilter::filterImage()
{
    if (d->radius < 1)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Radius out of range...";
        m_destImage = m_orgImage;

        return;
    }

    QList<int> vals = multithreadedSteps(m_orgImage.height());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                       &BlurFilter::blurMultithreaded, this,

#else

                                       this, &BlurFilter::blurMultithreaded,

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

FilterAction BlurFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("radius"), d->radius);

    return action;
}

void BlurFilter::readParameters(const FilterAction& action)
{
    d->radius = action.parameter(QLatin1String("radius")).toInt();
}

} // namespace Digikam

#include "moc_blurfilter.cpp"
