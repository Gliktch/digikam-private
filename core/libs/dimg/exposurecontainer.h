/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-01-12
 * Description : exposure indicator settings container.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExposureSettingsContainer
{

public:

    ExposureSettingsContainer()          = default;

    virtual ~ExposureSettingsContainer() = default;

public:

    bool   underExposureIndicator   = false;
    bool   overExposureIndicator    = false;

    /**
     * @brief If this option is true, over and under exposure indicators will be displayed
     * only when pure white and pure black color matches, as all color components match
     * the condition in the same time.
     * Else indicators are turn on when one of color components match the condition.
     */
    bool   exposureIndicatorMode    = true;

    float  underExposurePercent     = 1.0;
    float  overExposurePercent      = 1.0;

    QColor underExposureColor       = Qt::white;
    QColor overExposureColor        = Qt::black;

private:

    Q_DISABLE_COPY(ExposureSettingsContainer)
};

} // namespace Digikam
