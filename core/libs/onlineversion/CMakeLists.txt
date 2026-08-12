#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libonlineversion_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/onlineversionchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/onlineversiondlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/onlineversiondwnl.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_onlineversion_obj OBJECT ${libonlineversion_SRCS})

target_compile_definitions(core_onlineversion_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
