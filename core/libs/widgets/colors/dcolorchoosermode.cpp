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

#include "dcolorchoosermode_p.h"

// Qt includes

#include <QColor>

namespace Digikam
{

qreal getComponentValue(const QColor& color, DColorChooserMode chooserMode)
{
    switch (chooserMode)
    {
        case ChooserRed:
        {
            return color.redF();
        }

        case ChooserGreen:
        {
            return color.greenF();
        }

        case ChooserBlue:
        {
            return color.blueF();
        }

        case ChooserHue:
        {
            return color.hueF();
        }

        case ChooserSaturation:
        {
            return color.saturationF();
        }

        default:
        {
            return color.valueF();
        }
    }
}

void setComponentValue(QColor& color, DColorChooserMode chooserMode, qreal value)
{
    if (chooserMode >= ChooserRed)
    {
        if      (chooserMode == ChooserRed)
        {
            color.setRedF(value);
        }
        else if (chooserMode == ChooserGreen)
        {
            color.setGreenF(value);
        }
        else
        {
            // chooserMode == ChooserBlue
            color.setBlueF(value);
        }
    }
    else
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        float h = 0.0F, s = 0.0F, v = 0.0F, a = 0.0F;

#else

        qreal h = 0., s = 0., v = 0., a = 0.;

#endif

        color.getHsvF(&h, &s, &v, &a);

        if      (chooserMode == ChooserHue)
        {
            h = value;
        }
        else if (chooserMode == ChooserSaturation)
        {
            s = value;
        }
        else
        {
            // chooserMode == ChooserValue
            v = value;
        }

        color.setHsvF(h, s, v, a);
    }
}

} // namespace Digikam
