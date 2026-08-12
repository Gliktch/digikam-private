#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libtemplate_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/templatemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/templatelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/templateselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/templateviewer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/templatepanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/subjectedit.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_digikamtemplate_obj OBJECT  ${libtemplate_SRCS})

target_compile_definitions(gui_digikamtemplate_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
