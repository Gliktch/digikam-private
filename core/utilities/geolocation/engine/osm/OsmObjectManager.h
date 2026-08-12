/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class GeoDataPlacemark;

/**
 * @brief The OsmObjectManager class is used to assign osmData to placemarks that
 * do not have them at write time. Objects with no osmData usually consist of
 * new placemarks created in the editor and placemarks loaded from KML files.
 */
class DIGIKAM_EXPORT OsmObjectManager
{
public:

    /**
     * @brief initializeOsmData assigns valid osmData
     * to a placemark that does not have it.
     */
    static void initializeOsmData(GeoDataPlacemark* placemark);

    /**
     * @brief registerId is used to keep track of the minimum id @see m_minId
     */
    static void registerId(qint64 id);

private:

    /**
     * @brief newly created placemarks are assigned negative unique IDs.
     * In order to assure there are no duplicate IDs, they are assigned the
     * minId - 1 id.
     */
    static qint64 m_minId;
};

} // namespace Marble
