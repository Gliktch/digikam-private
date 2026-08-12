/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : video frame effects manager.
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

#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QImage>

// Local includes

#include "effectmngr.h"
#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN EffectMngr::Private
{
public:

    typedef int (EffectMngr::Private::*EffectMethod)(bool);

public:

    Private()
    {
        registerEffects();
    }

    ~Private() = default;

    QMap<EffectMngr::EffectType, EffectMethod>    eff_effectList;

    QImage                                        eff_image;
    QImage                                        eff_curFrame;
    QSize                                         eff_outSize;

    bool                                          eff_isRunning = false;
    EffectMngr::EffectType                        eff_curEffect = EffectMngr::None;

    int                                           eff_step      = 0;
    int                                           eff_imgFrames = 125;

public:

    void registerEffects();

    EffectMngr::EffectType getRandomEffect() const;

private:

    /**
     * Internal functions to render an effect frame.
     * The effect movement must be adjusted accordingly with amount of image frames to encode.
     * aInit is to true when effect is initialized (first call).
     * The returned integer value is a tempo in ms to wait between frames,
     * or -1 if the effect is completed.
     */

    int effectNone(bool aInit);
    int effectRandom(bool aInit);
    int effectKenBurnsZoomIn(bool aInit);
    int effectKenBurnsZoomOut(bool aInit);
    int effectKenBurnsPanLR(bool aInit);
    int effectKenBurnsPanRL(bool aInit);
    int effectKenBurnsPanTB(bool aInit);
    int effectKenBurnsPanBT(bool aInit);

    void updateCurrentFrame(const QRectF& area);
};

} // namespace Digikam
