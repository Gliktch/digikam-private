/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-05
 * Description : HotPixel filter settings container.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hotpixelcontainer.h"

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

HotPixelContainer::HotPixelContainer()
    : filterMethod(QUADRATIC_INTERPOLATION)
{
}

HotPixelContainer::~HotPixelContainer()
{
}

bool HotPixelContainer::isDefault() const
{
    return (*this == HotPixelContainer());
}

bool HotPixelContainer::operator==(const HotPixelContainer& other) const
{
    return (
            (blackFrameUrl == other.blackFrameUrl) &&
            (hotPixelsList == other.hotPixelsList) &&
            (filterMethod  == other.filterMethod)
           );
}

void HotPixelContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("blackframeurl"),  blackFrameUrl);
    action.addParameter(prefix + QLatin1String("hotpixelslist"),  HotPixelProps::toStringList(hotPixelsList));
    action.addParameter(prefix + QLatin1String("filtermethod"),   (int)filterMethod);
}

HotPixelContainer HotPixelContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    HotPixelContainer settings;
    settings.blackFrameUrl = action.parameter(prefix + QLatin1String("blackframeurl"),                               settings.blackFrameUrl);
    settings.hotPixelsList = HotPixelProps::fromStringList(action.parameter(prefix + QLatin1String("hotpixelslist"), QStringList()));
    settings.filterMethod  = (InterpolationMethod)action.parameter(prefix + QLatin1String("filtermethod"),           (int)settings.filterMethod);

    return settings;
}

} // namespace Digikam
