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

int TransitionMngr::Private::transitionSlideL2R(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0,     0, eff_outImage);
    bufferPainter.drawImage(eff_i, 0, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fx);

    if (eff_i <= eff_outSize.width())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionSlideR2L(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0,     0, eff_outImage);
    bufferPainter.drawImage(eff_i, 0, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fx);

    if (eff_i >= -eff_outSize.width())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionSlideT2B(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0,     eff_outImage);
    bufferPainter.drawImage(0, eff_i, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fy);

    if (eff_i <= eff_outSize.height())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionSlideB2T(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0,     eff_outImage);
    bufferPainter.drawImage(0, eff_i, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fy);

    if (eff_i >= -eff_outSize.height())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

} // namespace Digikam
