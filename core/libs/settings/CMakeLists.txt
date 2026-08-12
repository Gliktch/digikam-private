#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libsettings_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_albums.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_views.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_mime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_miscs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationsettings_tooltips.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_digikamsettings_obj OBJECT ${libsettings_SRCS})

target_compile_definitions(gui_digikamsettings_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
