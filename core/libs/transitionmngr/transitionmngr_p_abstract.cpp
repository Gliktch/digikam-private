/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : images transition manager.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "transitionmngr_p.h"

namespace Digikam
{

int TransitionMngr::Private::transitionMeltdown(bool aInit)
{
    int i;

    if (aInit)
    {
        delete [] eff_intArray;
        eff_w        = eff_outSize.width();
        eff_h        = eff_outSize.height();
        eff_dx       = 4;
        eff_dy       = 16;
        eff_ix       = eff_w / eff_dx;
        eff_intArray = new int[eff_ix];

        for (i = eff_ix - 1 ; i >= 0 ; --i)
        {
            eff_intArray[i] = 0;
        }
    }

    eff_pdone = true;

    int y, x;
    QPainter bufferPainter(&eff_curFrame);

    for (i = 0, x = 0 ; i < eff_ix ; ++i, x += eff_dx)
    {
        y = eff_intArray[i];

        if (y >= eff_h)
        {
            continue;
        }

        eff_pdone = false;

        if (randomGenerator->bounded(16U) < 6)
        {
            continue;
        }

        //bufferPainter.drawImage(x, y + eff_dy, eff_curFrame, x, y, eff_dx, eff_h - y - eff_dy);
        bufferPainter.drawImage(x, y, eff_outImage, x, y, eff_dx, eff_dy);

        eff_intArray[i] += eff_dy;
    }

    bufferPainter.end();

    if (eff_pdone)
    {
        delete [] eff_intArray;
        eff_intArray = nullptr;
        eff_curFrame = eff_outImage;
        return -1;
    }

    return 15;
}

int TransitionMngr::Private::transitionMosaic(bool aInit)
{
    int dim    = 10;         // Size of a cell (dim x dim)
    int margin = dim + (int)(dim / 4);

    if (aInit)
    {
        eff_i           = 30; // giri totally
        eff_pixelMatrix = new bool*[eff_outSize.width()];

        for (int x = 0 ; x < eff_outSize.width() ; ++x)
        {
            eff_pixelMatrix[x] = new bool[eff_outSize.height()];

            for (int y = 0 ; y < eff_outSize.height() ; ++y)
            {
                eff_pixelMatrix[x][y] = false;
            }
        }
    }

    if (eff_i <= 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    int w = eff_outSize.width();
    int h = eff_outSize.height();

    QPainter bufferPainter(&eff_curFrame);

    for (int x = 0 ; x < w ; x += randomGenerator->bounded(dim, dim+margin))
    {
        for (int y = 0 ; y < h ; y += randomGenerator->bounded(dim, dim+margin))
        {
            if (eff_pixelMatrix[x][y] == true)
            {
                if (y != 0)
                {
                    y--;
                }

                continue;
            }

            bufferPainter.fillRect(x, y, dim, dim, QBrush(eff_outImage));

            for (int i = 0 ; i < dim && (x + i) < w ; ++i)
            {
                for (int j = 0 ; j < dim && (y + j) < h ; ++j)
                {
                    eff_pixelMatrix[x+i][y+j] = true;
                }
            }
        }
    }

    bufferPainter.end();
    eff_i--;

    return 20;
}

int TransitionMngr::Private::transitionCubism(bool aInit)
{
    if (aInit)
    {
        eff_alpha = M_PI * 2;
        eff_w     = eff_outSize.width();
        eff_h     = eff_outSize.height();
        eff_i     = 150;
    }

    if (eff_i <= 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    QPainter bufferPainter(&eff_curFrame);

    eff_x   = randomGenerator->bounded(eff_w);
    eff_y   = randomGenerator->bounded(eff_h);
    int r   = randomGenerator->bounded(100, 200);
    eff_px  = eff_x - r;
    eff_py  = eff_y - r;
    eff_psx = r;
    eff_psy = r;

    QTransform transform;
    transform.rotate(randomGenerator->bounded(-10, 10));
    QRect rect(eff_px, eff_py, eff_psx, eff_psy);
    bufferPainter.setTransform(transform);
    bufferPainter.fillRect(rect, QBrush(eff_outImage));
    bufferPainter.end();

    eff_i--;

    return 10;
}

int TransitionMngr::Private::transitionBlobs(bool aInit)
{
    int r;

    if (aInit)
    {
        eff_alpha = M_PI * 2;
        eff_w     = eff_outSize.width();
        eff_h     = eff_outSize.height();
        eff_i     = 150;
    }

    if (eff_i <= 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    eff_x    = randomGenerator->bounded(eff_w);
    eff_y    = randomGenerator->bounded(eff_h);
    r        = randomGenerator->bounded(50, 250);
    eff_px   = eff_x - r;
    eff_py   = eff_y - r;
    eff_psx  = r;
    eff_psy  = r;

    QPainterPath painterPath;
    painterPath.addEllipse(eff_px, eff_py, eff_psx, eff_psy);
    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.fillPath(painterPath, QBrush(eff_outImage));
    bufferPainter.end();

    eff_i--;

    return 10;
}

} // namespace Digikam
