/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : border settings container.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT BorderContainer
{

public:

    enum BorderTypes
    {
        SolidBorder = 0,
        NiepceBorder,
        BeveledBorder,
        PineBorder,
        WoodBorder,
        PaperBorder,
        ParqueBorder,
        IceBorder,
        LeafBorder,
        MarbleBorder,
        RainBorder,
        CratersBorder,
        DriedBorder,
        PinkBorder,
        StoneBorder,
        ChalkBorder,
        GraniteBorder,
        RockBorder,
        WallBorder
    };

public:

    BorderContainer()  = default;
    ~BorderContainer() = default;

    static QString getBorderPath(int border);

public:

    bool    preserveAspectRatio     = true;

    int     orgWidth                = 0;
    int     orgHeight               = 0;

    int     borderType              = 0;

    int     borderWidth1            = 0;
    int     borderWidth2            = 0;
    int     borderWidth3            = 0;
    int     borderWidth4            = 0;

    double  borderPercent           = 0.1;

    QString borderPath;

    QColor  solidColor              = QColor(0, 0, 0);
    QColor  niepceBorderColor       = QColor(255, 255, 255);
    QColor  niepceLineColor         = QColor(0, 0, 0);
    QColor  bevelUpperLeftColor     = QColor(192, 192, 192);
    QColor  bevelLowerRightColor    = QColor(128, 128, 128);
    QColor  decorativeFirstColor    = QColor(0, 0, 0);
    QColor  decorativeSecondColor   = QColor(0, 0, 0);
};

} // namespace Digikam
