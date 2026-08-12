/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Guillaume Castagnino <casta at xwing dot info>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "wbcontainer.h"
#include "wbfilter.h"

namespace Digikam
{

bool WBContainer::isDefault() const
{
    return (*this == WBContainer());
}

bool WBContainer::operator==(const WBContainer& other) const
{
    return (
            (black          == other.black)          &&
            (expositionMain == other.expositionMain) &&
            (expositionFine == other.expositionFine) &&
            (temperature    == other.temperature)    &&
            (green          == other.green)          &&
            (dark           == other.dark)           &&
            (gamma          == other.gamma)          &&
            (saturation     == other.saturation)
           );
}

void WBContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("black"),          black);
    action.addParameter(prefix + QLatin1String("expositionMain"), expositionMain);
    action.addParameter(prefix + QLatin1String("expositionFine"), expositionFine);
    action.addParameter(prefix + QLatin1String("temperature"),    temperature);
    action.addParameter(prefix + QLatin1String("green"),          green);
    action.addParameter(prefix + QLatin1String("dark"),           dark);
    action.addParameter(prefix + QLatin1String("gamma"),          gamma);
    action.addParameter(prefix + QLatin1String("saturation"),     saturation);
}

WBContainer WBContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    WBContainer settings;
    settings.black          = action.parameter(prefix + QLatin1String("black"),          settings.black);
    settings.expositionMain = action.parameter(prefix + QLatin1String("expositionMain"), settings.expositionMain);
    settings.expositionFine = action.parameter(prefix + QLatin1String("expositionFine"), settings.expositionFine);
    settings.temperature    = action.parameter(prefix + QLatin1String("temperature"),    settings.temperature);
    settings.green          = action.parameter(prefix + QLatin1String("green"),          settings.green);
    settings.dark           = action.parameter(prefix + QLatin1String("dark"),           settings.dark);
    settings.gamma          = action.parameter(prefix + QLatin1String("gamma"),          settings.gamma);
    settings.saturation     = action.parameter(prefix + QLatin1String("saturation"),     settings.saturation);

    return settings;
}

} // namespace Digikam
