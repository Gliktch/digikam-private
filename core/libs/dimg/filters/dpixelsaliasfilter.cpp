/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : pixels antialiasing filter
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original channel mixer algorithm copyrighted 2002 by
 * Martin Guldahl <mguldahl at xmission dot com> from Gimp 2.2
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpixelsaliasfilter.h"

// C++ includes

#include <cstring>
#include <cstdlib>

// Local includes

#include "imagehistogram.h"
#include "dcolor.h"

namespace Digikam
{

DPixelsAliasFilter::DPixelsAliasFilter()
{
}

DPixelsAliasFilter::~DPixelsAliasFilter()
{
}

/**
 * @brief Function to perform pixel antialiasing with 8 bits/color/pixel images. This method is used to smooth target
 * image in transformation  method like free rotation or shear tool.
 */
void DPixelsAliasFilter::pixelAntiAliasing(const uchar* const data,
                                           int Width,
                                           int Height,
                                           double X,
                                           double Y,
                                           uchar* const A,
                                           uchar* const R,
                                           uchar* const G,
                                           uchar* const B)
{
    int nX, nY, j;
    double lfWeightX[2], lfWeightY[2], lfWeight;
    double lfTotalR = 0.0, lfTotalG = 0.0, lfTotalB = 0.0, lfTotalA = 0.0;

    nX = (int)X;
    nY = (int)Y;

    if (Y >= 0.0)
    {
        lfWeightY[0] = 1.0 - (lfWeightY[1] = Y - (double)nY);
    }
    else
    {
        lfWeightY[1] = 1.0 - (lfWeightY[0] = -(Y - (double)nY));
    }

    if (X >= 0.0)
    {
        lfWeightX[0] = 1.0 - (lfWeightX[1] = X - (double)nX);
    }
    else
    {
        lfWeightX[1] = 1.0 - (lfWeightX[0] = -(X - (double)nX));
    }

    for (int loopx = 0 ; loopx <= 1 ; ++loopx)
    {
        for (int loopy = 0 ; loopy <= 1 ; ++loopy)
        {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j = setPositionAdjusted(Width, Height, nX + loopx, nY + loopy);

            lfTotalB += ((double)data[j] * lfWeight);
            ++j;
            lfTotalG += ((double)data[j] * lfWeight);
            ++j;
            lfTotalR += ((double)data[j] * lfWeight);
            ++j;
            lfTotalA += ((double)data[j] * lfWeight);
            ++j;
        }
    }

    *B = CLAMP0255((int)lfTotalB);
    *G = CLAMP0255((int)lfTotalG);
    *R = CLAMP0255((int)lfTotalR);
    *A = CLAMP0255((int)lfTotalA);
}

/**
 * @brief Function to perform pixel antialiasing with 16 bits/color/pixel images. This method is used to smooth target
 * image in transformation  method like free rotation or shear tool.
 */
void DPixelsAliasFilter::pixelAntiAliasing16(const unsigned short* const data,
                                             int Width,
                                             int Height,
                                             double X,
                                             double Y,
                                             unsigned short* const A,
                                             unsigned short* const R,
                                             unsigned short* const G,
                                             unsigned short* const B)
{
    int nX, nY, j;
    double lfWeightX[2], lfWeightY[2], lfWeight;
    double lfTotalR = 0.0, lfTotalG = 0.0, lfTotalB = 0.0, lfTotalA = 0.0;

    nX = (int)X;
    nY = (int)Y;

    if (Y >= 0.0)
    {
        lfWeightY[0] = 1.0 - (lfWeightY[1] = Y - (double)nY);
    }
    else
    {
        lfWeightY[1] = 1.0 - (lfWeightY[0] = -(Y - (double)nY));
    }

    if (X >= 0.0)
    {
        lfWeightX[0] = 1.0 - (lfWeightX[1] = X - (double)nX);
    }
    else
    {
        lfWeightX[1] = 1.0 - (lfWeightX[0] = -(X - (double)nX));
    }

    for (int loopx = 0 ; loopx <= 1 ; ++loopx)
    {
        for (int loopy = 0 ; loopy <= 1 ; ++loopy)
        {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j        = setPositionAdjusted(Width, Height, nX + loopx, nY + loopy);

            lfTotalB += ((double)data[j] * lfWeight);
            ++j;
            lfTotalG += ((double)data[j] * lfWeight);
            ++j;
            lfTotalR += ((double)data[j] * lfWeight);
            ++j;
            lfTotalA += ((double)data[j] * lfWeight);
            ++j;
        }
    }

    *B = CLAMP065535((int)lfTotalB);
    *G = CLAMP065535((int)lfTotalG);
    *R = CLAMP065535((int)lfTotalR);
    *A = CLAMP065535((int)lfTotalA);
}

inline int DPixelsAliasFilter::setPositionAdjusted(int Width,
                                                   int Height,
                                                   int X,
                                                   int Y)
{
    X = (X < 0) ? 0 : (X >= Width)  ? Width  - 1 : X;
    Y = (Y < 0) ? 0 : (Y >= Height) ? Height - 1 : Y;

    return (Y * Width * 4 + 4 * X);
}

} // namespace Digikam
