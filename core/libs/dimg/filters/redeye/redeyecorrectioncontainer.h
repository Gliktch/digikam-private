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

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT RedEyeCorrectionContainer
{

public:

    RedEyeCorrectionContainer() = default;

    bool isDefault()                                            const;
    bool operator==(const RedEyeCorrectionContainer& other)     const;

    void writeToFilterAction(FilterAction& action,
                             const QString& prefix = QString()) const;

    static RedEyeCorrectionContainer fromFilterAction(const FilterAction& action,
                                                      const QString& prefix = QString());

public:

    double m_redToAvgRatio = 2.1;
};

} // namespace Digikam
