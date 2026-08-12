/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-15
 * Description : white balance color correction settings container
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Guillaume Castagnino <casta at xwing dot info>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

class DIGIKAM_EXPORT WBContainer
{

public:

    WBContainer() = default;

    bool   isDefault()                                            const;
    bool   operator==(const WBContainer& other)                   const;

    void   writeToFilterAction(FilterAction& action,
                               const QString& prefix = QString()) const;

    static WBContainer fromFilterAction(const FilterAction& action,
                                        const QString& prefix = QString());

public:

    /**
     * Neutral color temperature settings.
     */

    double black            = 0.0;
    double expositionMain   = 0.0;
    double expositionFine   = 0.0;
    double temperature      = 6500.0;
    double green            = 1.0;
    double dark             = 0.0;
    double gamma            = 1.0;
    double saturation       = 1.0;
};

} // namespace Digikam
