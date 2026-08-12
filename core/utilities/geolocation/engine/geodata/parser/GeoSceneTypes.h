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

// Local includes

#include "digikam_export.h"

namespace Marble
{

namespace GeoSceneTypes
{

/**
 * Please keep alphabetic order to prevent mess
 */
DIGIKAM_EXPORT extern const char GeoSceneDocumentType[];
DIGIKAM_EXPORT extern const char GeoSceneFilterType[];
DIGIKAM_EXPORT extern const char GeoSceneGeodataType[];
DIGIKAM_EXPORT extern const char GeoSceneGroupType[];
DIGIKAM_EXPORT extern const char GeoSceneHeadType[];
DIGIKAM_EXPORT extern const char GeoSceneIconType[];
DIGIKAM_EXPORT extern const char GeoSceneItemType[];
DIGIKAM_EXPORT extern const char GeoSceneLayerType[];
DIGIKAM_EXPORT extern const char GeoSceneLegendType[];
DIGIKAM_EXPORT extern const char GeoSceneLicenseType[];
DIGIKAM_EXPORT extern const char GeoSceneMapType[];
DIGIKAM_EXPORT extern const char GeoScenePaletteType[];
DIGIKAM_EXPORT extern const char GeoScenePropertyType[];
DIGIKAM_EXPORT extern const char GeoSceneSectionType[];
DIGIKAM_EXPORT extern const char GeoSceneSettingsType[];
DIGIKAM_EXPORT extern const char GeoSceneTextureTileType[];
DIGIKAM_EXPORT extern const char GeoSceneTileDatasetType[];
DIGIKAM_EXPORT extern const char GeoSceneVectorType[];
DIGIKAM_EXPORT extern const char GeoSceneVectorTileType[];
DIGIKAM_EXPORT extern const char GeoSceneXmlDataSourceType[];
DIGIKAM_EXPORT extern const char GeoSceneZoomType[];

} // namespace GeoSceneTypes

} // namespace Marble
