/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Primitive datatypes for geolocation interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPersistentModelIndex>

Q_DECLARE_METATYPE(QPersistentModelIndex)

namespace Digikam
{

enum GeoMouseMode
{
    MouseModePan                     = 1,
    MouseModeRegionSelection         = 2,
    MouseModeRegionSelectionFromIcon = 4,
    MouseModeFilter                  = 8,
    MouseModeSelectThumbnail         = 16,
    MouseModeZoomIntoGroup           = 32,
    MouseModeLast                    = 32
};

Q_DECLARE_FLAGS(GeoMouseModes, GeoMouseMode)

enum GeoExtraAction
{
    ExtraActionSticky = 1,
    ExtraLoadTracks   = 2
};

Q_DECLARE_FLAGS(GeoExtraActions, GeoExtraAction)

typedef QList<int> QIntList;

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoMouseModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoExtraActions)

Q_DECLARE_METATYPE(Digikam::GeoMouseModes)
