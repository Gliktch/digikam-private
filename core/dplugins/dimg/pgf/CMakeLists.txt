#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_SOURCE_DIR}/core/libs/pgfutils/libpgf
)

set(dimgpgfplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgpgfplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgpgfloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgpgfloader_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgpgfloader_save.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgpgfexportsettings.cpp
)

DIGIKAM_ADD_DIMG_PLUGIN(NAME    PGF
                        SOURCES ${dimgpgfplugin_SRCS}
)
