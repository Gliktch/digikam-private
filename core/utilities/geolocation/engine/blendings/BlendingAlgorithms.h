/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>

// Local includes

#include "Blending.h"

namespace Marble
{

class OverpaintBlending: public Blending
{
public:

    void blend(QImage* const bottom, TextureTile const* const top) const override;
};

class IndependentChannelBlending: public Blending
{
public:

    void blend(QImage* const bottom, TextureTile const* const top) const override;

private:

    // bottomColorIntensity: intensity of one color channel (of one pixel) of the bottom image
    // topColorIntensity: intensity of one color channel (of one pixel) of the top image
    // return: intensity of the color channel (of a given pixel) of the result image
    // all color intensity values are in the range 0..1
    virtual qreal blendChannel(qreal const bottomColorIntensity,
                               qreal const topColorIntensity) const = 0;
};

// Neutral blendings

class AllanonBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class ArcusTangentBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class GeometricMeanBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class LinearLightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class NoiseBlending: public Blending // or IndependentChannelBlending?
{
};

class OverlayBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class ParallelBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class TextureBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

// Darkening blendings

class ColorBurnBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class DarkBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class DarkenBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class DivideBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class GammaDarkBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class LinearBurnBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class MultiplyBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class SubtractiveBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

// Lightning blendings

class AdditiveBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class ColorDodgeBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class GammaLightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class HardLightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class LightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class LightenBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class PinLightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class ScreenBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class SoftLightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class VividLightBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

// Inverter blendings

class AdditiveSubtractiveBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class BleachBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class DifferenceBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class EquivalenceBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

class HalfDifferenceBlending: public IndependentChannelBlending
{
    qreal blendChannel(qreal const bottomColorIntensity,
                       qreal const topColorIntensity) const override;
};

// Special purpose blendings

class CloudsBlending: public Blending
{
public:

    void blend(QImage* const bottom, TextureTile const* const top) const override;
};

class GrayscaleBlending: public Blending
{
public:

    void blend(QImage* const bottom, TextureTile const* const top) const override;
};

class InvertColorBlending: public Blending
{
public:

    void blend(QImage* const bottom, TextureTile const* const top) const override;
};

class InvertHueBlending: public Blending
{
public:

    void blend(QImage* const bottom, TextureTile const* const top) const override;
};

} // Namespace Marble
