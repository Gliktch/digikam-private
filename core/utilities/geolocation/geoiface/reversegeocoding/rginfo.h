/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : Reverse geocoding data.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPersistentModelIndex>
#include <QMap>

// Local includes

#include "geoifacetypes.h"
#include "gpsdatacontainer.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * @class RGInfo
 *
 * @brief This class contains data needed in reverse geocoding process.
 */
class DIGIKAM_EXPORT RGInfo
{
public:

    /**
     * Constructor
     */
    RGInfo()  = default;

    /**
     * Destructor
     */
    ~RGInfo() = default;

public:

    /**
     * The image index.
     */
    QPersistentModelIndex  id;

    /**
     * The coordinates of current image.
     */
    GeoCoordinates         coordinates;

    /**
     * The address elements and their names.
     */
    QMap<QString, QString> rgData;
};

} // namespace Digikam
