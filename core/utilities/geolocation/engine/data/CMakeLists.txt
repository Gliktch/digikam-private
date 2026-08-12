#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
set (MARBLE_DATA_INSTALL_PATH ${KDE_INSTALL_FULL_DATADIR}/digikam/marble/)

install (FILES
         legend.html
         legend.css
         DESTINATION ${MARBLE_DATA_INSTALL_PATH})

FILE (GLOB LEGEND_FILES *.leg)
install (FILES ${LEGEND_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH})

FILE (GLOB BITMAP_FILES bitmaps/*.png)
install (FILES ${BITMAP_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/bitmaps)

INSTALL (DIRECTORY svg/osmcarto/svg
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/svg/osmcarto
         FILES_MATCHING PATTERN "*.svg"
         PATTERN "svgorig" EXCLUDE)

INSTALL (DIRECTORY svg/material svg/thenounproject
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/svg
         FILES_MATCHING PATTERN "*.svg")

FILE (GLOB BITMAP_EDITARROW_FILES bitmaps/editarrows/*.png)
install (FILES ${BITMAP_EDITARROW_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/bitmaps/editarrows)

install (DIRECTORY bitmaps/osmcarto
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/bitmaps
         PATTERN "LICENSE.txt" EXCLUDE)

FILE (GLOB BITMAP_STAR_FILES bitmaps/stars/*.png)
install (FILES ${BITMAP_STAR_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/bitmaps/stars)

FILE (GLOB BITMAP_FILES bitmaps/*.jpg)
install (FILES ${BITMAP_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/bitmaps)

install (FILES
         svg/marble-logo.svg
         svg/marble-logo.png
         svg/marble-logo-32dpi.png
         svg/marble-logo-72dpi.png
         svg/marble-logo-inverted-32dpi.png
         svg/marble-logo-inverted-72dpi.png
         svg/application-x-marble.svg
         svg/application-x-marble.png
         svg/application-x-marble-gray.png
         svg/worldmap.svg
         svg/lunarmap.svg
         svg/marsmap.svg
         svg/coordinate.svg
         svg/wikipedia.svg
         svg/wikipedia_shadow.svg
         svg/sunshine.png
         svg/sun.png
         svg/moon.png
         svg/glow.png
         svg/track_turtle.svg
         svg/dot-circle-regular.svg
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/svg)

FILE (GLOB SVG_FILES flags/*.svg)
install (FILES ${SVG_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/flags)

install (FILES
         maps/earth/citylights/citylights.jpg
         maps/earth/citylights/citylights.dgml
         maps/earth/citylights/citylights-preview.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/citylights)

install (FILES
         maps/earth/citylights/legend/city.png
         maps/earth/citylights/legend/lightterrain.png
         maps/earth/citylights/legend/darkterrain.png
         maps/earth/citylights/legend/water.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/citylights/legend)

install (FILES
         maps/earth/clouds/0/000000/000000_000000.jpg
         maps/earth/clouds/0/000000/000000_000001.jpg
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/clouds/0/000000)

install (DIRECTORY maps/earth/srtm
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth)

install (FILES
         maps/earth/srtm2/srtm2.dgml
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/srtm2)

install (FILES
         maps/earth/srtm2/0/000000/000000_000000.png
         maps/earth/srtm2/0/000000/000000_000001.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/srtm2/0/000000)

install (FILES
         maps/earth/bluemarble/bluemarble.jpg
         maps/earth/bluemarble/bluemarble.dgml
         maps/earth/bluemarble/bluemarble-preview.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/bluemarble)

install (FILES
         maps/earth/bluemarble/legend/ice.png
         maps/earth/bluemarble/legend/mountain.png
         maps/earth/bluemarble/legend/vegetation.png
         maps/earth/bluemarble/legend/desert.png
         maps/earth/bluemarble/legend/water.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/bluemarble/legend)

install (FILES
         maps/earth/plain/plain.dgml
         maps/earth/plain/plain-preview.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/plain)

install (DIRECTORY maps/earth/openstreetmap
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth)

install (FILES
         maps/earth/humanitarian/0/0/0.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/humanitarian/0/0)

install (FILES
         maps/earth/hillshading/0/0/0.png
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth/hillshading/0/0)

install (DIRECTORY maps/earth/vectorosm
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/maps/earth)

macro( marble_add_project_resources resources )
    add_custom_target( ${PROJECT_NAME}_Resources ALL SOURCES ${ARGN} )
endmacro()

FILE (GLOB_RECURSE DGML_FILES maps/*.dgml)
      marble_add_project_resources(${DGML_FILES})

install (FILES
         weather/bbc-stations.xml
         weather/weather-clear-night.png
         weather/weather-clear.png
         weather/weather-clouds-night.png
         weather/weather-clouds.png
         weather/weather-few-clouds-night.png
         weather/weather-few-clouds.png
         weather/weather-hail.png
         weather/weather-many-clouds.png
         weather/weather-mist.png
         weather/weather-none-available.png
         weather/weather-showers-day.png
         weather/weather-showers-night.png
         weather/weather-showers-scattered-day.png
         weather/weather-showers-scattered-night.png
         weather/weather-showers-scattered.png
         weather/weather-showers.png
         weather/weather-snow-rain.png
         weather/weather-snow-scattered-day.png
         weather/weather-snow-scattered-night.png
         weather/weather-snow-scattered.png
         weather/weather-snow.png
         weather/weather-storm-day.png
         weather/weather-storm-night.png
         weather/weather-storm.png
         weather/wind-arrows.svgz
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/weather)

install (FILES
         mwdbii/DATELINE.PNT
         mwdbii/PCOAST.PNT
         mwdbii/PDIFFBORDER.PNT
         mwdbii/PGLACIER.PNT
         mwdbii/PISLAND.PNT
         mwdbii/PLAKEISLAND.PNT
         mwdbii/PLAKE.PNT
         mwdbii/PUSA48.DIFF.PNT
         mwdbii/RIVER.PNT
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/mwdbii)

FILE (GLOB PN2_FILES naturalearth/*.pn2)
install (FILES ${PN2_FILES}
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/naturalearth)

FILE (GLOB STAR_FILES stars/*.dat)
install (FILES ${STAR_FILES} stars/deepsky.png stars/names.csv
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/stars)

install (FILES
         placemarks/elevplacemarks.cache
         placemarks/otherplacemarks.cache
         placemarks/baseplacemarks.cache
         placemarks/moonlandingsites.cache
         placemarks/boundaryplacemarks.cache
         placemarks/moonterrain.cache
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/placemarks)

install (FILES placemarks/cityplacemarks.cache
         DESTINATION ${MARBLE_DATA_INSTALL_PATH}/placemarks)
