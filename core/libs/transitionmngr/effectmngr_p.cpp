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

#include "effectmngr_p.h"

// Qt includes

#include <QRandomGenerator>

namespace Digikam
{

void EffectMngr::Private::registerEffects()
{
    eff_effectList.insert(EffectMngr::None,            &EffectMngr::Private::effectNone);
    eff_effectList.insert(EffectMngr::KenBurnsZoomIn,  &EffectMngr::Private::effectKenBurnsZoomIn);
    eff_effectList.insert(EffectMngr::KenBurnsZoomOut, &EffectMngr::Private::effectKenBurnsZoomOut);
    eff_effectList.insert(EffectMngr::KenBurnsPanLR,   &EffectMngr::Private::effectKenBurnsPanLR);
    eff_effectList.insert(EffectMngr::KenBurnsPanRL,   &EffectMngr::Private::effectKenBurnsPanRL);
    eff_effectList.insert(EffectMngr::KenBurnsPanTB,   &EffectMngr::Private::effectKenBurnsPanTB);
    eff_effectList.insert(EffectMngr::KenBurnsPanBT,   &EffectMngr::Private::effectKenBurnsPanBT);
}

EffectMngr::EffectType EffectMngr::Private::getRandomEffect() const
{
    QList<EffectMngr::EffectType> effs = eff_effectList.keys();
    effs.removeAt(effs.indexOf(EffectMngr::None));

    int i = QRandomGenerator::global()->bounded(effs.count());

    return effs[i];
}

void EffectMngr::Private::updateCurrentFrame(const QRectF& area)
{
    QImage kbImg = eff_image.copy(area.toAlignedRect())
                            .scaled(eff_outSize,
                                    Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation);
    eff_curFrame = kbImg.convertToFormat(QImage::Format_ARGB32);
}

int EffectMngr::Private::effectRandom(bool /*aInit*/)
{
    return -1;
}

int EffectMngr::Private::effectNone(bool)
{
    eff_curFrame = eff_image;

    return -1;
}

} // namespace Digikam
