#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libfuzzysearch_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzzysearchview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzzysearchview_sketch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzzysearchview_similar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/findduplicatesview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/findduplicatesalbum.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/findduplicatesalbumitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sketchwidget.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Used by digikamgui
add_library(gui_fuzzysearch_obj OBJECT ${libfuzzysearch_SRCS})

target_compile_definitions(gui_fuzzysearch_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
