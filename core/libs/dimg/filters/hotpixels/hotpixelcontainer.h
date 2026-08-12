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

#pragma once

// Qt includes

#include <QString>
#include <QUrl>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "hotpixelprops.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT HotPixelContainer
{

public:

    enum InterpolationMethod
    {
        AVERAGE_INTERPOLATION   = 0,
        LINEAR_INTERPOLATION    = 1,
        QUADRATIC_INTERPOLATION = 2,
        CUBIC_INTERPOLATION     = 3
    };

    enum Direction
    {
        TWODIM_DIRECTION        = 0,
        VERTICAL_DIRECTION      = 1,
        HORIZONTAL_DIRECTION    = 2
    };

public:

    explicit HotPixelContainer();
    ~HotPixelContainer();

    bool isDefault() const;
    bool operator==(const HotPixelContainer& other)                                   const;

    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;

    static HotPixelContainer fromFilterAction(const FilterAction& action,
                                              const QString& prefix = QString());

public:

    QUrl                 blackFrameUrl;
    QList<HotPixelProps> hotPixelsList;
    InterpolationMethod  filterMethod;
};

} // namespace Digikam
