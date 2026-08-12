/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-15
 * Description : a Brightness/Contrast/Gamma settings container.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "digikam_globals.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT BCGContainer
{

public:

    BCGContainer() = default;

    bool isDefault()                                                                  const;
    bool operator==(const BCGContainer& other)                                        const;

    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;

    static BCGContainer fromFilterAction(const FilterAction& action,
                                         const QString& prefix = QString());

public:

    int    channel    = LuminosityChannel;

    double brightness = 0.0;
    double contrast   = 0.0;
    double gamma      = 1.0;
};

} // namespace Digikam
