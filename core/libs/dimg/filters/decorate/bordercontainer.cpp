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

#include "bordercontainer.h"

// Qt includes

#include <QStandardPaths>

namespace Digikam
{

QString BorderContainer::getBorderPath(int border)
{
    QString pattern;

    switch (border)
    {
        case BorderContainer::PineBorder:
        {
            pattern = QLatin1String("pine-pattern");
            break;
        }

        case BorderContainer::WoodBorder:
        {
            pattern = QLatin1String("wood-pattern");
            break;
        }

        case BorderContainer::PaperBorder:
        {
            pattern = QLatin1String("paper-pattern");
            break;
        }

        case BorderContainer::ParqueBorder:
        {
            pattern = QLatin1String("parque-pattern");
            break;
        }

        case BorderContainer::IceBorder:
        {
            pattern = QLatin1String("ice-pattern");
            break;
        }

        case BorderContainer::LeafBorder:
        {
            pattern = QLatin1String("leaf-pattern");
            break;
        }

        case BorderContainer::MarbleBorder:
        {
            pattern = QLatin1String("marble-pattern");
            break;
        }

        case BorderContainer::RainBorder:
        {
            pattern = QLatin1String("rain-pattern");
            break;
        }

        case BorderContainer::CratersBorder:
        {
            pattern = QLatin1String("craters-pattern");
            break;
        }

        case BorderContainer::DriedBorder:
        {
            pattern = QLatin1String("dried-pattern");
            break;
        }

        case BorderContainer::PinkBorder:
        {
            pattern = QLatin1String("pink-pattern");
            break;
        }

        case BorderContainer::StoneBorder:
        {
            pattern = QLatin1String("stone-pattern");
            break;
        }

        case BorderContainer::ChalkBorder:
        {
            pattern = QLatin1String("chalk-pattern");
            break;
        }

        case BorderContainer::GraniteBorder:
        {
            pattern = QLatin1String("granit-pattern");
            break;
        }

        case BorderContainer::RockBorder:
        {
            pattern = QLatin1String("rock-pattern");
            break;
        }

        case BorderContainer::WallBorder:
        {
            pattern = QLatin1String("wall-pattern");
            break;
        }

        default:
        {
            return pattern;
        }
    }

    return (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/") + pattern + QLatin1String(".webp")));
}

} // namespace Digikam
