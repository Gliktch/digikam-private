/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-20
 * Description : color chooser widgets
 *
 * SPDX-FileCopyrightText:      2010 by Christoph Feck <christoph at maxiom dot de>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dcolorchoosermode.h"

// Qt includes

#include <QtGlobal>

class QColor;

namespace Digikam
{

/**
 * get/set color component
 */
extern qreal getComponentValue(const QColor& color, DColorChooserMode chooserMode);
extern void  setComponentValue(QColor& color, DColorChooserMode chooserMode, qreal value);

/**
 * number of linear gradient ranges needed for color component
 */
static inline int componentValueSteps(DColorChooserMode chooserMode)
{
    if (chooserMode == ChooserHue)
    {
        return 6;
    }
    else
    {
        return 1;
    }
}

/**
 * color component that is used for X in the XY selector
 */
static inline DColorChooserMode chooserXMode(DColorChooserMode chooserMode)
{
    if (chooserMode >= ChooserRed)
    {
        return (chooserMode == ChooserRed ? ChooserGreen : ChooserRed);
    }
    else
    {
        return (chooserMode == ChooserHue ? ChooserSaturation : ChooserHue);
    }
}

/**
 * color component that is used for Y in the XY selector
 */
static inline DColorChooserMode chooserYMode(DColorChooserMode chooserMode)
{
    if (chooserMode >= ChooserRed)
    {
        return (chooserMode == ChooserBlue ? ChooserGreen : ChooserBlue);
    }
    else
    {
        return (chooserMode == ChooserValue ? ChooserSaturation : ChooserValue);
    }
}

static inline int componentXSteps(DColorChooserMode chooserMode)
{
    return componentValueSteps(chooserXMode(chooserMode));
}

static inline int componentYSteps(DColorChooserMode chooserMode)
{
    return componentValueSteps(chooserYMode(chooserMode));
}

static inline qreal getComponentX(const QColor& color, DColorChooserMode chooserMode)
{
    return getComponentValue(color, chooserXMode(chooserMode));
}

static inline qreal getComponentY(const QColor& color, DColorChooserMode chooserMode)
{
    return getComponentValue(color, chooserYMode(chooserMode));
}

static inline void setComponentX(QColor& color, DColorChooserMode chooserMode, qreal x)
{
    setComponentValue(color, chooserXMode(chooserMode), x);
}

static inline void setComponentY(QColor& color, DColorChooserMode chooserMode, qreal y)
{
    setComponentValue(color, chooserYMode(chooserMode), y);
}

} // namespace Digikam
