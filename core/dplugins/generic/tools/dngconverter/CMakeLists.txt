#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

set(dngconverterplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dngconverterplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngconverterlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngconvertertask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngconverterthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngconverterdialog.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    DNGConverter
                           SOURCES ${dngconverterplugin_SRCS}
)
