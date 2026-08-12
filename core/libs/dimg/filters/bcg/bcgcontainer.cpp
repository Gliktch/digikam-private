/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bcgcontainer.h"

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

bool BCGContainer::isDefault() const
{
    return (*this == BCGContainer());
}

bool BCGContainer::operator==(const BCGContainer& other) const
{
    return (
            (channel    == other.channel)    &&
            (brightness == other.brightness) &&
            (contrast   == other.contrast)   &&
            (gamma      == other.gamma)
           );
}

void BCGContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("channel"),    channel);
    action.addParameter(prefix + QLatin1String("brightness"), brightness);
    action.addParameter(prefix + QLatin1String("contrast"),   contrast);
    action.addParameter(prefix + QLatin1String("gamma"),      gamma);
}

BCGContainer BCGContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    BCGContainer settings;
    settings.channel    = action.parameter(prefix + QLatin1String("channel"),    settings.channel);
    settings.brightness = action.parameter(prefix + QLatin1String("brightness"), settings.brightness);
    settings.contrast   = action.parameter(prefix + QLatin1String("contrast"),   settings.contrast);
    settings.gamma      = action.parameter(prefix + QLatin1String("gamma"),      settings.gamma);

    return settings;
}

} // namespace Digikam
