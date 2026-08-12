#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_subdirectory(cache)                         # Cache Files Loader
add_subdirectory(gpsbabel)                      # GPSBabel formats converter to KML
add_subdirectory(gpx)                           # GPX data loader
add_subdirectory(json)                          # JSON data loader
add_subdirectory(kml)                           # KML data loader
add_subdirectory(nominatim-reversegeocoding)    # OSM reverse geo-coding
add_subdirectory(osm)                           # OSM vectorized data loader
add_subdirectory(pn2)                           # PN2 data loader
add_subdirectory(pnt)                           # PNT data loader
