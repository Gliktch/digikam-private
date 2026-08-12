#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/dialog
                    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport
                    ${CMAKE_CURRENT_SOURCE_DIR}/searches
)

set(geolocationeditplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchresultbackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchresultmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchresultmodelhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchresultwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog/gpsitemdetails.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog/geolocationedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/geodataparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/kmlgpsdataparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/kmlwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/kmlexport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/geolocationeditplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    GeolocationEdit
                           SOURCES ${geolocationeditplugin_SRCS}
                           DEPENDS Qt${QT_VERSION_MAJOR}::Concurrent
)
