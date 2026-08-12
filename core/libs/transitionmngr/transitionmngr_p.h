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

#pragma once

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QRandomGenerator>

// Local includes

#include "transitionmngr.h"
#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN TransitionMngr::Private
{
public:

    typedef int (TransitionMngr::Private::*TransMethod)(bool);

public:

    Private()
    {
        registerTransitions();
    }

    ~Private()
    {
        if (eff_intArray)
        {
            delete [] eff_intArray;
        }
    }


    QMap<TransitionMngr::TransType, TransMethod>  eff_transList;

    QImage                                        eff_inImage;
    QImage                                        eff_outImage;
    QImage                                        eff_curFrame;
    QSize                                         eff_outSize;

    bool                                          eff_isRunning         = false;
    TransitionMngr::TransType                     eff_curTransition     = TransitionMngr::None;

    // values for state of various transitions:
    int                                           eff_x                 = 0;
    int                                           eff_y                 = 0;
    int                                           eff_w                 = 0;
    int                                           eff_h                 = 0;
    int                                           eff_dx                = 0;
    int                                           eff_dy                = 0;
    int                                           eff_ix                = 0;
    int                                           eff_iy                = 0;
    int                                           eff_i                 = 0;
    int                                           eff_j                 = 0;
    int                                           eff_subType           = 0;
    int                                           eff_x0                = 0;
    int                                           eff_y0                = 0;
    int                                           eff_x1                = 0;
    int                                           eff_y1                = 0;
    int                                           eff_wait              = 0;
    double                                        eff_fx                = 0;
    double                                        eff_fy                = 0;
    double                                        eff_alpha             = 0;
    double                                        eff_fd                = 0;

    int*                                          eff_intArray          = nullptr;
    bool                                          eff_pdone             = false;
    bool**                                        eff_pixelMatrix       = nullptr;

    //static
    QPolygon                                      eff_pa                = QPolygon(4);

    int                                           eff_px                = 0;
    int                                           eff_py                = 0;
    int                                           eff_psx               = 0;
    int                                           eff_psy               = 0;

public:

    void registerTransitions();

    TransitionMngr::TransType getRandomTransition() const;

private:

    // Internal functions to render an effect frame.
    // aInit is to true when effect is initialized (first call).
    // The returned integer value is a tempo in ms to wait between frames,
    // or -1 if the effect is completed.

    int transitionNone(bool aInit);
    int transitionChessboard(bool aInit);
    int transitionMeltdown(bool aInit);
    int transitionSweep(bool aInit);
    int transitionMosaic(bool aInit);
    int transitionCubism(bool aInit);
    int transitionRandom(bool aInit);
    int transitionGrowing(bool aInit);
    int transitionHorizLines(bool aInit);
    int transitionVertLines(bool aInit);
    int transitionMultiCircleOut(bool aInit);
    int transitionSpiralIn(bool aInit);
    int transitionCircleOut(bool aInit);
    int transitionBlobs(bool aInit);
    int transitionFade(bool aInit);
    int transitionSlideL2R(bool aInit);
    int transitionSlideR2L(bool aInit);
    int transitionSlideT2B(bool aInit);
    int transitionSlideB2T(bool aInit);
    int transitionPushL2R(bool aInit);
    int transitionPushR2L(bool aInit);
    int transitionPushT2B(bool aInit);
    int transitionPushB2T(bool aInit);
    int transitionSwapL2R(bool aInit);
    int transitionSwapR2L(bool aInit);
    int transitionSwapT2B(bool aInit);
    int transitionSwapB2T(bool aInit);
    int transitionBlurIn(bool aInit);
    int transitionBlurOut(bool aInit);

private:

    QRgb   convertFromPremult(const QRgb& p)       const;
    QImage fastBlur(const QImage& img, int radius) const;

private:

    QRandomGenerator* randomGenerator = QRandomGenerator::global();
};

} // namespace Digikam
