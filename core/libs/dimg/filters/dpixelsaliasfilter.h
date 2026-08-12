/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : pixels antialiasing filter
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <cmath>

// Local includes

#include "digikam_export.h"
#include "digikam_globals.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPixelsAliasFilter
{
public:

    explicit DPixelsAliasFilter();
    ~DPixelsAliasFilter();

public:   // Public methods.

    void pixelAntiAliasing(const uchar* const data,
                           int Width,
                           int Height,
                           double X,
                           double Y,
                           uchar* const A,
                           uchar* const R,
                           uchar* const G,
                           uchar* const B);

    void pixelAntiAliasing16(const unsigned short* const data,
                             int Width,
                             int Height,
                             double X,
                             double Y,
                             unsigned short* const A,
                             unsigned short* const R,
                             unsigned short* const G,
                             unsigned short* const B);

private:

    inline int setPositionAdjusted(int Width,
                                   int Height,
                                   int X,
                                   int Y);
};

} // namespace Digikam
