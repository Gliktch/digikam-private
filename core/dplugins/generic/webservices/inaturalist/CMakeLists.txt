#
# SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)

# --------------------------------------------------------------------------------

set(inatplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/inatplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inatscore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inatsuggest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inattalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inattaxon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inattaxonedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inatwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inatwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inatutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inatbrowserdlg.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    iNaturalist
                           SOURCES ${inatplugin_SRCS}
                           DEPENDS ${QWEB_BACKEND}

)

add_subdirectory(icons)
