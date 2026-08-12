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

namespace GeoDataTypes
{

/**
 * the following const char* XXType are used to provide RTTI to the GeoData for
 * downcasting to the good object
 * please keep alphabetic order to prevent mess
 */
DIGIKAM_EXPORT extern const char GeoDataAliasType[];
DIGIKAM_EXPORT extern const char GeoDataAnimatedUpdateType[];
DIGIKAM_EXPORT extern const char GeoDataCameraType[];
DIGIKAM_EXPORT extern const char GeoDataChangeType[];
DIGIKAM_EXPORT extern const char GeoDataColorStyleType[];
DIGIKAM_EXPORT extern const char GeoDataCreateType[];
DIGIKAM_EXPORT extern const char GeoDataDataType[];
DIGIKAM_EXPORT extern const char GeoDataDeleteType[];
DIGIKAM_EXPORT extern const char GeoDataDocumentType[];
DIGIKAM_EXPORT extern const char GeoDataExtendedDataType[];
DIGIKAM_EXPORT extern const char GeoDataFlyToType[];
DIGIKAM_EXPORT extern const char GeoDataFolderType[];
DIGIKAM_EXPORT extern const char GeoDataGroundOverlayType[];
DIGIKAM_EXPORT extern const char GeoDataHotspotType[];
DIGIKAM_EXPORT extern const char GeoDataIconStyleType[];
DIGIKAM_EXPORT extern const char GeoDataInnerBoundaryType[];
DIGIKAM_EXPORT extern const char GeoDataLabelStyleType[];
DIGIKAM_EXPORT extern const char GeoDataLatLonAltBoxType[];
DIGIKAM_EXPORT extern const char GeoDataLatLonBoxType[];
DIGIKAM_EXPORT extern const char GeoDataLatLonQuadType[];
DIGIKAM_EXPORT extern const char GeoDataLinearRingType[];
DIGIKAM_EXPORT extern const char GeoDataLineStringType[];
DIGIKAM_EXPORT extern const char GeoDataLineStyleType[];
DIGIKAM_EXPORT extern const char GeoDataLinkType[];
DIGIKAM_EXPORT extern const char GeoDataLocationType[];
DIGIKAM_EXPORT extern const char GeoDataLodType[];
DIGIKAM_EXPORT extern const char GeoDataLookAtType[];
DIGIKAM_EXPORT extern const char GeoDataModelType[];
DIGIKAM_EXPORT extern const char GeoDataMultiGeometryType[];
DIGIKAM_EXPORT extern const char GeoDataMultiTrackType[];
DIGIKAM_EXPORT extern const char GeoDataNetworkLinkType[];
DIGIKAM_EXPORT extern const char GeoDataOrientationType[];
DIGIKAM_EXPORT extern const char GeoDataOuterBoundaryType[];
DIGIKAM_EXPORT extern const char GeoDataPhotoOverlayType[];
DIGIKAM_EXPORT extern const char GeoDataPlacemarkType[];
DIGIKAM_EXPORT extern const char GeoDataPlaylistType[];
DIGIKAM_EXPORT extern const char GeoDataPointType[];
DIGIKAM_EXPORT extern const char GeoDataPolygonType[];
DIGIKAM_EXPORT extern const char GeoDataPolyStyleType[];
DIGIKAM_EXPORT extern const char GeoDataRegionType[];
DIGIKAM_EXPORT extern const char GeoDataRelationType[];
DIGIKAM_EXPORT extern const char GeoDataResourceMapType[];
DIGIKAM_EXPORT extern const char GeoDataSchemaType[];
DIGIKAM_EXPORT extern const char GeoDataSchemaDataType[];
DIGIKAM_EXPORT extern const char GeoDataSimpleDataType[];
DIGIKAM_EXPORT extern const char GeoDataSimpleFieldType[];
DIGIKAM_EXPORT extern const char GeoDataSimpleArrayDataType[];
DIGIKAM_EXPORT extern const char GeoDataStyleType[];
DIGIKAM_EXPORT extern const char GeoDataStyleMapType[];
DIGIKAM_EXPORT extern const char GeoDataSoundCueType[];
DIGIKAM_EXPORT extern const char GeoDataTimePrimitiveType[];
DIGIKAM_EXPORT extern const char GeoDataTimeStampType[];
DIGIKAM_EXPORT extern const char GeoDataTimeSpanType[];
DIGIKAM_EXPORT extern const char GeoDataTourType[];
DIGIKAM_EXPORT extern const char GeoDataTourControlType[];
DIGIKAM_EXPORT extern const char GeoDataWaitType[];
DIGIKAM_EXPORT extern const char GeoDataTrackType[];
DIGIKAM_EXPORT extern const char GeoDataScaleType[];
DIGIKAM_EXPORT extern const char GeoDataScreenOverlayType[];
DIGIKAM_EXPORT extern const char GeoDataBalloonStyleType[];
DIGIKAM_EXPORT extern const char GeoDataListStyleType[];
DIGIKAM_EXPORT extern const char GeoDataItemIconType[];
DIGIKAM_EXPORT extern const char GeoDataImagePyramidType[];
DIGIKAM_EXPORT extern const char GeoDataViewVolumeType[];
DIGIKAM_EXPORT extern const char GeoDataNetworkLinkControlType[];
DIGIKAM_EXPORT extern const char GeoDataUpdateType[];
DIGIKAM_EXPORT extern const char GeoDataBuildingType[];

} // namespace GeoDataTypes

} // namespace Marble
