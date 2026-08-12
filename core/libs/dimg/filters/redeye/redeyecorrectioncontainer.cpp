/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-15
 * Description : Red Eyes auto correction settings container.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "redeyecorrectioncontainer.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

bool RedEyeCorrectionContainer::isDefault() const
{
    return (*this == RedEyeCorrectionContainer());
}

bool RedEyeCorrectionContainer::operator==(const RedEyeCorrectionContainer& other) const
{
    return (m_redToAvgRatio == other.m_redToAvgRatio);
}

void RedEyeCorrectionContainer::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    action.addParameter(prefix + QLatin1String("redtoavgratio"), m_redToAvgRatio);
}

RedEyeCorrectionContainer RedEyeCorrectionContainer::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    RedEyeCorrectionContainer settings;
    settings.m_redToAvgRatio = action.parameter(prefix + QLatin1String("redtoavgratio"), settings.m_redToAvgRatio);

    return settings;
}

} // namespace Digikam
