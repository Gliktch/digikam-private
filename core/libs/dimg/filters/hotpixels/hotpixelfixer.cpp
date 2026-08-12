/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hotpixelfixer.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QColor>
#include <QStringList>

// Local includes

#include "dimg.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

#ifdef HAVE_FLOAT_H
#   if HAVE_FLOAT_H
#       include <float.h>
#   endif
#endif

namespace Digikam
{

#ifndef DBL_MIN
    static const double DBL_MIN = 1e-10;
#endif

HotPixelFixer::HotPixelFixer(QObject* const parent)
    : DImgThreadedFilter(parent)

{
    initFilter();
}

HotPixelFixer::HotPixelFixer(DImg* const orgImage,
                             QObject* const parent,
                             const HotPixelContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("HotPixels")),
      m_settings        (settings)
{
    initFilter();
}

HotPixelFixer::~HotPixelFixer()
{
    cancelFilter();
}

QString HotPixelFixer::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Hot Pixels Tool"));
}

Digikam::FilterAction HotPixelFixer::filterAction()
{
    DefaultFilterAction<HotPixelFixer> action;
    action.addParameter(QLatin1String("interpolationMethod"), m_settings.filterMethod);

    for (const HotPixelProps& hp : std::as_const(m_settings.hotPixelsList))
    {
        action.addParameter(QLatin1String("hotPixel"), hp.toString());
    }

    return action;
}

void HotPixelFixer::readParameters(const FilterAction& action)
{
    m_settings.filterMethod = (HotPixelContainer::InterpolationMethod)action.parameter(QLatin1String("interpolationMethod")).toInt();

    const QHash<QString, QVariant> &params = action.parameters();

    for (QHash<QString, QVariant>::const_iterator i = params.constBegin(); i != params.constEnd(); ++i)
    {
        const QString &key = i.key();

        if (key == QLatin1String("hotPixel"))
        {
            HotPixelProps hp;

            if (hp.fromString(i.value().toString()))
            {
                m_settings.hotPixelsList << hp;
            }
        }
    }
}

void HotPixelFixer::filterImage()
{
    QList <HotPixelProps>::ConstIterator it;
    QList <HotPixelProps>::ConstIterator end(m_settings.hotPixelsList.constEnd());

    for (it = m_settings.hotPixelsList.constBegin() ; it != end ; ++it)
    {
        HotPixelProps hp = *it;
        interpolate(m_orgImage, hp, m_settings.filterMethod);
    }

    m_destImage = m_orgImage;
}

/**
 * Interpolates a pixel block
 */
void HotPixelFixer::interpolate(DImg& img, const HotPixelProps& hp, int method)
{
    const int xPos = hp.x();
    const int yPos = hp.y();
    bool sixtBits  = img.sixteenBit();

    // Interpolate pixel.

    switch (method)
    {
        case HotPixelContainer::AVERAGE_INTERPOLATION:
        {
            // We implement the bidimendional one first.
            // TODO: implement the rest of directions (V & H) here

            //case twodim:
            // {
            int sum_weight = 0;
            double vr      = 0.0;
            double vg      = 0.0;
            double vb      = 0.0;
            int x, y;
            DColor col;

            for (x = xPos ; x < xPos+hp.width() ; ++x)
            {
                if (validPoint(img, QPoint(x, yPos - 1)))
                {
                    col = img.getPixelColor(x, yPos - 1);
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }

                if (validPoint(img, QPoint(x, yPos + hp.height())))
                {
                    col = img.getPixelColor(x, yPos + hp.height());
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
            }

            for (y = yPos ; y < hp.height() ; ++y)
            {
                if (validPoint(img, QPoint(xPos - 1, y)))
                {
                    col = img.getPixelColor(xPos, y);
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }

                if (validPoint(img, QPoint(xPos + hp.width(), y)))
                {
                    col = img.getPixelColor(xPos + hp.width(), y);
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
            }

            if (sum_weight > 0)
            {
                vr /= (double)sum_weight;
                vg /= (double)sum_weight;
                vb /= (double)sum_weight;

                for (x = 0 ; x < hp.width() ; ++x)
                {
                    for (y = 0 ; y < hp.height() ; ++y)
                    {
                        if (validPoint(img, QPoint(xPos + x, yPos + y)))
                        {
                            int alpha = sixtBits ? 65535 : 255;
                            int ir    = (int)round(vr);
                            int ig    = (int)round(vg);
                            int ib    = (int)round(vb);
                            img.setPixelColor(xPos + x, yPos + y, DColor(ir, ig, ib, alpha, sixtBits));
                        }
                    }
                }
            }

            break;
        }

        case HotPixelContainer::LINEAR_INTERPOLATION:
        {
            weightPixels(img, hp, HotPixelContainer::LINEAR_INTERPOLATION, HotPixelContainer::TWODIM_DIRECTION, sixtBits ? 65535: 255);
            break;
        }

        case HotPixelContainer::QUADRATIC_INTERPOLATION:
        {
            weightPixels(img, hp, HotPixelContainer::QUADRATIC_INTERPOLATION, HotPixelContainer::TWODIM_DIRECTION, sixtBits ? 65535 : 255);
            break;
        }

        case HotPixelContainer::CUBIC_INTERPOLATION:
        {
            weightPixels(img, hp, HotPixelContainer::CUBIC_INTERPOLATION, HotPixelContainer::TWODIM_DIRECTION, sixtBits ? 65535 : 255);
            break;
        }
    }
}

void HotPixelFixer::weightPixels(DImg& img,
                                 const HotPixelProps& px,
                                 int method,
                                 HotPixelContainer::Direction dir,
                                 int maxComponent)
{
    // TODO: implement direction here too

    for (int iComp = 0 ; iComp < 3 ; ++iComp)
    {
        // Obtain weight data block.

        HotPixelsWeights w;
        int polynomeOrder = -1;

        switch (method)
        {
            case HotPixelContainer::LINEAR_INTERPOLATION:
            {
                polynomeOrder = 1;
                break;
            }

            case HotPixelContainer::QUADRATIC_INTERPOLATION:
            {
                polynomeOrder = 2;
                break;
            }

            case HotPixelContainer::CUBIC_INTERPOLATION:
            {
                polynomeOrder = 3;
                break;
            }

            default:
            {
                return; // Not supported.
            }
        }

        if (polynomeOrder < 0)
        {
            return;
        }

        // In the one-dimensional case, the width must be 1,
        // and the size must be stored in height

        w.setWidth(dir == HotPixelContainer::TWODIM_DIRECTION ? px.width() : 1);
        w.setHeight(dir == HotPixelContainer::HORIZONTAL_DIRECTION ? px.width() : px.height());
        w.setPolynomeOrder(polynomeOrder);
        w.setTwoDim(dir == HotPixelContainer::TWODIM_DIRECTION);

        // TODO: check this, it must not recalculate existing calculated weights
        // for now I don't think it is finding the duplicates fine, so it uses
        // the previous one always...

        //if (mWeightList.find(w)==mWeightList.end())
        //{
        w.calculateHotPixelsWeights();

        //    mWeightList.append(w);

        //}

        // Calculate weighted pixel sum.

        for (int y = 0 ; y < px.height() ; ++y)
        {
            for (int x = 0 ; x < px.width() ; ++x)
            {
                if (validPoint(img, QPoint(px.x() + x, px.y() + y)))
                {
                    double sum_weight = 0.0;
                    double v          = 0.0;
                    size_t i;

                    for (i = 0 ; i < (size_t)w.positions().count() ; ++i)
                    {
                        // In the one-dimensional case, only the y coordinate is used.

                        const int xx = px.x() + ((dir == HotPixelContainer::VERTICAL_DIRECTION) ? x
                                                                                                : (dir == HotPixelContainer::HORIZONTAL_DIRECTION) ? w.positions().at((int)i).y()
                                                                                                                                                   : w.positions().at((int)i).x());
                        const int yy = px.y() + ((dir == HotPixelContainer::HORIZONTAL_DIRECTION) ? y
                                                                                                  : w.positions().at((int)i).y());

                        if (validPoint (img,QPoint(xx, yy)))
                        {
                            // TODO: check this. I think it is broken

                            double weight;

                            if      (dir == HotPixelContainer::VERTICAL_DIRECTION)
                            {
                                weight = w[(int)i][y][0];
                            }
                            else if (dir == HotPixelContainer::HORIZONTAL_DIRECTION)
                            {
                                weight = w[(int)i][0][x];
                            }
                            else
                            {
                                weight = w[(int)i][y][x];
                            }

                            if      (iComp == 0)
                            {
                                v += weight * img.getPixelColor(xx, yy).red();
                            }
                            else if (iComp == 1)
                            {
                                v += weight * img.getPixelColor(xx, yy).green();
                            }
                            else
                            {
                                v += weight * img.getPixelColor(xx, yy).blue();
                            }

                            sum_weight += weight;
                        }
                    }

                    DColor color = img.getPixelColor(px.x()+x,px.y()+y);
                    int component;

                    if      (fabs (v) <= DBL_MIN)
                    {
                        component = 0;
                    }
                    else if (sum_weight > DBL_MIN)
                    {
                        component = (int) (v / sum_weight);

                        // Clamp value

                        if (component < 0)
                        {
                            component = 0;
                        }

                        if (component > maxComponent)
                        {
                            component = maxComponent;
                        }
                    }
                    else
                    {
                        component = (v >= 0.0) ? maxComponent : 0;
                    }

                    if      (iComp == 0)
                    {
                        color.setRed(component);
                    }
                    else if (iComp == 1)
                    {
                        color.setGreen(component);
                    }
                    else
                    {
                        color.setBlue(component);
                    }

                    img.setPixelColor(px.x() + x, px.y() + y,color);
                }
            }
        }
    }
}

} // namespace Digikam

#include "moc_hotpixelfixer.cpp"
