#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

if(${ENABLE_GEOLOCATION})

    include_directories(${MARBLE_INCLUDE_DIR})

endif()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)

set(libgeoiface_SRCS
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/track_correlator.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/track_correlator_thread.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/track_listmodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/gpscorrelatorwidget.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/rgtagmodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/rgwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/simpletreemodel.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/tracks/trackreader.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tracks/trackmanager.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/lookup/lookupaltitude.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/lookup/lookupaltitudegeonames.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/lookup/lookupfactory.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/dragdrop/mapdragdrophandler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/dragdrop/gpsitemlistdragdrophandler.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemcontainer.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemdelegate.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemmodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemsortproxymodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemlist.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemlistcontextmenu.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geodragdrophandler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geocoordinates.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geoifacecommon.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geomodelhelper.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geopluginaboutdlg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geolocationsettings.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geolocationsettingscontainer.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/groupstatecomputer.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/gpscommon.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/gpsundocommand.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/gpsgeoifacemodelhelper.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/gpsbookmarkowner.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/gpsbookmarkmodelhelper.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarknode.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarksmenu.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarksmngr.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarksdlg.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/abstractmarkertiler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/itemmarkertiler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/tilegrouper.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/tileindex.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-geonames-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-geonamesUS-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-osm-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/mapbackend.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backendgooglemaps.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/placeholderwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget_settings.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget_map.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget_data.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget_setup.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget_appearance.cpp
)

if(${ENABLE_GEOLOCATION})

    set(libgeoiface_SRCS ${libgeoiface_SRCS}

                         # TODO: files to port to Qt 6 when Marble will be ready

                         ${CMAKE_CURRENT_SOURCE_DIR}/backends/backendmarble.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/backends/backendmarblelayer.cpp
    )

endif()

set(libgeoiface_SRCS
    ${libgeoiface_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/htmlwidget_qwebengine.cpp
)

if(NOT Qt6_FOUND)

    # Marble translations need explicit loading
    ecm_create_qm_loader(libgeoiface_SRCS marble_qt)

endif()

# Used by digikamcore
add_library(core_geoiface_obj OBJECT ${libgeoiface_SRCS})

target_compile_definitions(core_geoiface_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
