/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image curves manipulation methods.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPolygon>
#include <QString>

// Local includes

#include "digikam_globals.h"
#include "digikam_export.h"
#include "imagecurves.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT CurvesContainer
{

public:

    /**
     * Provides a convenient storage for a curve.
     * Initially, the values are empty.
     * Call initialize() before adjusting values manually.
     */
    CurvesContainer() = default;
    CurvesContainer(int type, bool sixteenBit);

    /**
     * Fills the values with a linear curve suitable for type and sixteenBit parameters.
     */
    void initialize();

    /**
     * An empty container is interpreted as a linear curve.
     * A non-empty container can also be linear; test for isLinear()
     * of the resulting ImageCurves.
     * Note: If an ImageCurves is linear, it will return an empty container.
     */
    bool isEmpty()                                                                      const;

    bool operator==(const CurvesContainer& other)                                       const;

    /**
     * Serialize from and to FilterAction.
     * isStoredLosslessly returns false if the curve cannot be losslessly stored
     * in XML because it would be too large (free 16 bit). It is then lossily compressed.
     */
    bool isStoredLosslessly()                                                           const;
    void writeToFilterAction(FilterAction& action, const QString& prefix = QString())   const;
    static CurvesContainer fromFilterAction(const FilterAction& action, const QString& prefix = QString());

public:

    /**
     *  Smooth : QPolygon have size of 18 points.
     *  Free   : QPolygon have size of 255 or 65535 values.
     */
    int                     curvesType = ImageCurves::CURVE_SMOOTH;
    QPolygon                values[ColorChannels];

    bool                    sixteenBit = false;
};

} // namespace Digikam
