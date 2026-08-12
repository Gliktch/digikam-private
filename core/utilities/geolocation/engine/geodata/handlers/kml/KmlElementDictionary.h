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

// Lists all known KML 2.1 tags (https://developers.google.com/kml/documentation/kmlreferencel)
namespace kml
{

DIGIKAM_EXPORT extern const char kmlTag_nameSpace20[];
DIGIKAM_EXPORT extern const char kmlTag_nameSpace21[];
DIGIKAM_EXPORT extern const char kmlTag_nameSpace22[];
DIGIKAM_EXPORT extern const char kmlTag_nameSpaceOgc22[];
DIGIKAM_EXPORT extern const char kmlTag_nameSpaceGx22[];
DIGIKAM_EXPORT extern const char kmlTag_nameSpaceMx[];

DIGIKAM_EXPORT extern const char kmlTag_address[];
DIGIKAM_EXPORT extern const char kmlTag_AddressDetails[];
DIGIKAM_EXPORT extern const char kmlTag_Alias[];
DIGIKAM_EXPORT extern const char kmlTag_altitude[];
DIGIKAM_EXPORT extern const char kmlTag_altitudeMode[];
DIGIKAM_EXPORT extern const char kmlTag_BalloonStyle[];
DIGIKAM_EXPORT extern const char kmlTag_begin[];
DIGIKAM_EXPORT extern const char kmlTag_bgColor[];
DIGIKAM_EXPORT extern const char kmlTag_bottomFov[];
DIGIKAM_EXPORT extern const char kmlTag_Camera[];
DIGIKAM_EXPORT extern const char kmlTag_Change[];
DIGIKAM_EXPORT extern const char kmlTag_code[];
DIGIKAM_EXPORT extern const char kmlTag_color[];
DIGIKAM_EXPORT extern const char kmlTag_colorMode[];
DIGIKAM_EXPORT extern const char kmlTag_ColorStyle[];
DIGIKAM_EXPORT extern const char kmlTag_Container[];
DIGIKAM_EXPORT extern const char kmlTag_cookie[];
DIGIKAM_EXPORT extern const char kmlTag_coordinates[];
DIGIKAM_EXPORT extern const char kmlTag_Create[];
DIGIKAM_EXPORT extern const char kmlTag_Data[];
DIGIKAM_EXPORT extern const char kmlTag_Delete[];
DIGIKAM_EXPORT extern const char kmlTag_description[];
DIGIKAM_EXPORT extern const char kmlTag_displayMode[];
DIGIKAM_EXPORT extern const char kmlTag_displayName[];
DIGIKAM_EXPORT extern const char kmlTag_Document[];
DIGIKAM_EXPORT extern const char kmlTag_drawOrder[];
DIGIKAM_EXPORT extern const char kmlTag_duration[];
DIGIKAM_EXPORT extern const char kmlTag_east[];
DIGIKAM_EXPORT extern const char kmlTag_end[];
DIGIKAM_EXPORT extern const char kmlTag_expires[];
DIGIKAM_EXPORT extern const char kmlTag_ExtendedData[];
DIGIKAM_EXPORT extern const char kmlTag_extrude[];
DIGIKAM_EXPORT extern const char kmlTag_Feature[];
DIGIKAM_EXPORT extern const char kmlTag_fill[];
DIGIKAM_EXPORT extern const char kmlTag_flyToMode[];
DIGIKAM_EXPORT extern const char kmlTag_flyToView[];
DIGIKAM_EXPORT extern const char kmlTag_Folder[];
DIGIKAM_EXPORT extern const char kmlTag_geomColor[];
DIGIKAM_EXPORT extern const char kmlTag_Geometry[];
DIGIKAM_EXPORT extern const char kmlTag_GeometryCollection[];
DIGIKAM_EXPORT extern const char kmlTag_geomScale[];
DIGIKAM_EXPORT extern const char kmlTag_gridOrigin[];
DIGIKAM_EXPORT extern const char kmlTag_GroundOverlay[];
DIGIKAM_EXPORT extern const char kmlTag_h[];
DIGIKAM_EXPORT extern const char kmlTag_heading[];
DIGIKAM_EXPORT extern const char kmlTag_height[];
DIGIKAM_EXPORT extern const char kmlTag_href[];
DIGIKAM_EXPORT extern const char kmlTag_hotSpot[];
DIGIKAM_EXPORT extern const char kmlTag_httpQuery[];
DIGIKAM_EXPORT extern const char kmlTag_Icon[];
DIGIKAM_EXPORT extern const char kmlTag_IconStyle[];
DIGIKAM_EXPORT extern const char kmlTag_ImagePyramid[];
DIGIKAM_EXPORT extern const char kmlTag_innerBoundaryIs[];
DIGIKAM_EXPORT extern const char kmlTag_ItemIcon[];
DIGIKAM_EXPORT extern const char kmlTag_key[];
DIGIKAM_EXPORT extern const char kmlTag_kml[];
DIGIKAM_EXPORT extern const char kmlTag_labelColor[];
DIGIKAM_EXPORT extern const char kmlTag_LabelStyle[];
DIGIKAM_EXPORT extern const char kmlTag_latitude[];
DIGIKAM_EXPORT extern const char kmlTag_LatLonAltBox[];
DIGIKAM_EXPORT extern const char kmlTag_LatLonBox[];
DIGIKAM_EXPORT extern const char kmlTag_LatLonQuad[];
DIGIKAM_EXPORT extern const char kmlTag_leftFov[];
DIGIKAM_EXPORT extern const char kmlTag_LinearRing[];
DIGIKAM_EXPORT extern const char kmlTag_LineString[];
DIGIKAM_EXPORT extern const char kmlTag_LineStyle[];
DIGIKAM_EXPORT extern const char kmlTag_Link[];
DIGIKAM_EXPORT extern const char kmlTag_linkDescription[];
DIGIKAM_EXPORT extern const char kmlTag_linkName[];
DIGIKAM_EXPORT extern const char kmlTag_linkSnippet[];
DIGIKAM_EXPORT extern const char kmlTag_listItemType[];
DIGIKAM_EXPORT extern const char kmlTag_ListStyle[];
DIGIKAM_EXPORT extern const char kmlTag_Location[];
DIGIKAM_EXPORT extern const char kmlTag_Lod[];
DIGIKAM_EXPORT extern const char kmlTag_longitude[];
DIGIKAM_EXPORT extern const char kmlTag_LookAt[];
DIGIKAM_EXPORT extern const char kmlTag_maxAltitude[];
DIGIKAM_EXPORT extern const char kmlTag_maxFadeExtent[];
DIGIKAM_EXPORT extern const char kmlTag_maxHeight[];
DIGIKAM_EXPORT extern const char kmlTag_maxLodPixels[];
DIGIKAM_EXPORT extern const char kmlTag_maxSessionLength[];
DIGIKAM_EXPORT extern const char kmlTag_maxWidth[];
DIGIKAM_EXPORT extern const char kmlTag_message[];
DIGIKAM_EXPORT extern const char kmlTag_Metadata[];
DIGIKAM_EXPORT extern const char kmlTag_minAltitude[];
DIGIKAM_EXPORT extern const char kmlTag_minFadeExtent[];
DIGIKAM_EXPORT extern const char kmlTag_minLodPixels[];
DIGIKAM_EXPORT extern const char kmlTag_minRefreshPeriod[];
DIGIKAM_EXPORT extern const char kmlTag_Model[];
DIGIKAM_EXPORT extern const char kmlTag_MultiGeometry[];
DIGIKAM_EXPORT extern const char kmlTag_name[];
DIGIKAM_EXPORT extern const char kmlTag_near[];
DIGIKAM_EXPORT extern const char kmlTag_NetworkLink[];
DIGIKAM_EXPORT extern const char kmlTag_NetworkLinkControl[];
DIGIKAM_EXPORT extern const char kmlTag_north[];
DIGIKAM_EXPORT extern const char kmlTag_Object[];
DIGIKAM_EXPORT extern const char kmlTag_ObjArrayField[];
DIGIKAM_EXPORT extern const char kmlTag_ObjField[];
DIGIKAM_EXPORT extern const char kmlTag_open[];
DIGIKAM_EXPORT extern const char kmlTag_Orientation[];
DIGIKAM_EXPORT extern const char kmlTag_OsmPlacemarkData[];
DIGIKAM_EXPORT extern const char kmlTag_member[];
DIGIKAM_EXPORT extern const char kmlTag_nd[];
DIGIKAM_EXPORT extern const char kmlTag_tag[];
DIGIKAM_EXPORT extern const char kmlTag_outerBoundaryIs[];
DIGIKAM_EXPORT extern const char kmlTag_outline[];
DIGIKAM_EXPORT extern const char kmlTag_Overlay[];
DIGIKAM_EXPORT extern const char kmlTag_overlayXY[];
DIGIKAM_EXPORT extern const char kmlTag_Pair[];
DIGIKAM_EXPORT extern const char kmlTag_phoneNumber[];
DIGIKAM_EXPORT extern const char kmlTag_PhotoOverlay[];
DIGIKAM_EXPORT extern const char kmlTag_Placemark[];
DIGIKAM_EXPORT extern const char kmlTag_Point[];
DIGIKAM_EXPORT extern const char kmlTag_Polygon[];
DIGIKAM_EXPORT extern const char kmlTag_PolyStyle[];
DIGIKAM_EXPORT extern const char kmlTag_range[];
DIGIKAM_EXPORT extern const char kmlTag_refreshInterval[];
DIGIKAM_EXPORT extern const char kmlTag_refreshMode[];
DIGIKAM_EXPORT extern const char kmlTag_refreshVisibility[];
DIGIKAM_EXPORT extern const char kmlTag_Region[];
DIGIKAM_EXPORT extern const char kmlTag_request[];
DIGIKAM_EXPORT extern const char kmlTag_ResourceMap[];
DIGIKAM_EXPORT extern const char kmlTag_Response[];
DIGIKAM_EXPORT extern const char kmlTag_rightFov[];
DIGIKAM_EXPORT extern const char kmlTag_roll[];
DIGIKAM_EXPORT extern const char kmlTag_rotation[];
DIGIKAM_EXPORT extern const char kmlTag_rotationXY[];
DIGIKAM_EXPORT extern const char kmlTag_Scale[];
DIGIKAM_EXPORT extern const char kmlTag_scale[];
DIGIKAM_EXPORT extern const char kmlTag_Schema[];
DIGIKAM_EXPORT extern const char kmlTag_SchemaData[];
DIGIKAM_EXPORT extern const char kmlTag_SchemaField[];
DIGIKAM_EXPORT extern const char kmlTag_ScreenOverlay[];
DIGIKAM_EXPORT extern const char kmlTag_screenXY[];
DIGIKAM_EXPORT extern const char kmlTag_shape[];
DIGIKAM_EXPORT extern const char kmlTag_SimpleArrayData[];
DIGIKAM_EXPORT extern const char kmlTag_SimpleArrayField[];
DIGIKAM_EXPORT extern const char kmlTag_SimpleField[];
DIGIKAM_EXPORT extern const char kmlTag_SimpleData[];
DIGIKAM_EXPORT extern const char kmlTag_size[];
DIGIKAM_EXPORT extern const char kmlTag_Snippet[];
DIGIKAM_EXPORT extern const char kmlTag_south[];
DIGIKAM_EXPORT extern const char kmlTag_sourceHref[];
DIGIKAM_EXPORT extern const char kmlTag_state[];
DIGIKAM_EXPORT extern const char kmlTag_Status[];
DIGIKAM_EXPORT extern const char kmlTag_Style[];
DIGIKAM_EXPORT extern const char kmlTag_StyleMap[];
DIGIKAM_EXPORT extern const char kmlTag_StyleSelector[];
DIGIKAM_EXPORT extern const char kmlTag_styleUrl[];
DIGIKAM_EXPORT extern const char kmlTag_targetHref[];
DIGIKAM_EXPORT extern const char kmlTag_tessellate[];
DIGIKAM_EXPORT extern const char kmlTag_text[];
DIGIKAM_EXPORT extern const char kmlTag_textColor[];
DIGIKAM_EXPORT extern const char kmlTag_tileSize[];
DIGIKAM_EXPORT extern const char kmlTag_tilt[];
DIGIKAM_EXPORT extern const char kmlTag_TimeSpan[];
DIGIKAM_EXPORT extern const char kmlTag_TimeStamp[];
DIGIKAM_EXPORT extern const char kmlTag_topFov[];
DIGIKAM_EXPORT extern const char kmlTag_Update[];
DIGIKAM_EXPORT extern const char kmlTag_Url[];
DIGIKAM_EXPORT extern const char kmlTag_value[];
DIGIKAM_EXPORT extern const char kmlTag_viewBoundScale[];
DIGIKAM_EXPORT extern const char kmlTag_viewFormat[];
DIGIKAM_EXPORT extern const char kmlTag_viewRefreshMode[];
DIGIKAM_EXPORT extern const char kmlTag_viewRefreshTime[];
DIGIKAM_EXPORT extern const char kmlTag_ViewVolume[];
DIGIKAM_EXPORT extern const char kmlTag_visibility[];
DIGIKAM_EXPORT extern const char kmlTag_w[];
DIGIKAM_EXPORT extern const char kmlTag_west[];
DIGIKAM_EXPORT extern const char kmlTag_when[];
DIGIKAM_EXPORT extern const char kmlTag_width[];
DIGIKAM_EXPORT extern const char kmlTag_x[];
DIGIKAM_EXPORT extern const char kmlTag_y[];
DIGIKAM_EXPORT extern const char kmlTag_z[];


// Google KML Extension Namespace (https://developers.google.com/kml/documentation/kmlreference#kml-extension-namespace-and-the-gx-prefix)
DIGIKAM_EXPORT extern const char kmlTag_AnimatedUpdate[];
DIGIKAM_EXPORT extern const char kmlTag_FlyTo[];
DIGIKAM_EXPORT extern const char kmlTag_MultiTrack[];
DIGIKAM_EXPORT extern const char kmlTag_Track[];
DIGIKAM_EXPORT extern const char kmlTag_coord[];
DIGIKAM_EXPORT extern const char kmlTag_delayedStart[];
DIGIKAM_EXPORT extern const char kmlTag_SoundCue[];
DIGIKAM_EXPORT extern const char kmlTag_Tour[];
DIGIKAM_EXPORT extern const char kmlTag_TourControl[];
DIGIKAM_EXPORT extern const char kmlTag_Wait[];
DIGIKAM_EXPORT extern const char kmlTag_Playlist[];
DIGIKAM_EXPORT extern const char kmlTag_playMode[];
DIGIKAM_EXPORT extern const char kmlTag_balloonVisibility[];

// preliminary implementation for the extra defined Tags

DIGIKAM_EXPORT extern const char kmlTag_MarblePlacemark[];
DIGIKAM_EXPORT extern const char kmlTag_area[];
DIGIKAM_EXPORT extern const char kmlTag_pop[];
DIGIKAM_EXPORT extern const char kmlTag_role[];
DIGIKAM_EXPORT extern const char kmlTag_CountryNameCode[];
DIGIKAM_EXPORT extern const char kmlTag_countrycode[];

} // namespace kml

// Helper macros
#define KML_DEFINE_TAG_HANDLER_20(Name) GEODATA_DEFINE_TAG_HANDLER(kml, Kml, Name, kmlTag_nameSpace20)
#define KML_DEFINE_TAG_HANDLER_21(Name) GEODATA_DEFINE_TAG_HANDLER(kml, Kml, Name, kmlTag_nameSpace21)
#define KML_DEFINE_TAG_HANDLER_22(Name) GEODATA_DEFINE_TAG_HANDLER(kml, Kml, Name, kmlTag_nameSpace22)
#define KML_DEFINE_TAG_HANDLER_OGC22(Name) GEODATA_DEFINE_TAG_HANDLER(kml, Kml, Name, kmlTag_nameSpaceOgc22)
#define KML_DEFINE_TAG_HANDLER_GX22(Name) GEODATA_DEFINE_TAG_HANDLER(kml, Kml, Name, kmlTag_nameSpaceGx22)
#define KML_DEFINE_TAG_HANDLER_MX(Name) GEODATA_DEFINE_TAG_HANDLER(kml, Kml, Name, kmlTag_nameSpaceMx)

#define KML_DEFINE_TAG_HANDLER(Name) \
    KML_DEFINE_TAG_HANDLER_20(Name) \
    KML_DEFINE_TAG_HANDLER_21(Name) \
    KML_DEFINE_TAG_HANDLER_22(Name) \
    KML_DEFINE_TAG_HANDLER_OGC22(Name)

} // namespace Marble
