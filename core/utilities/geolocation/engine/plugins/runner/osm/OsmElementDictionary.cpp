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

#include "OsmElementDictionary.h"

namespace Marble
{

namespace osm
{

// OSM data primitives and what they represent
// https://wiki.openstreetmap.org/wiki/Elements

const char osmTag_version06[] = "0.6";

// KML to OSM translators

const char osmTranslate_document[] = "document";
const char osmTranslate_feature[] = "feature";
const char osmTranslate_placemark[] = "placemark";

// OSM writers

const char osmTag_osm[] = "osm";
const char osmTag_bound[] = "bound";
const char osmTag_bounds[] = "bounds";
const char osmTag_nd[] = "nd";
const char osmTag_node[] = "node";
const char osmTag_way[] = "way";
const char osmTag_relation[] = "relation";
const char osmTag_member[] = "member";
const char osmTag_tag[] = "tag";

} // namespace osm

} // namespace Marble
