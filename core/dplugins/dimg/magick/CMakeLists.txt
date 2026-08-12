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
)

set(dimgimagemagickplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgimagemagickplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgimagemagickloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgimagemagickloader_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgimagemagickloader_save.cpp
)

DIGIKAM_ADD_DIMG_PLUGIN(NAME    ImageMagick
                        SOURCES ${dimgimagemagickplugin_SRCS}
                        DEPENDS ${ImageMagick_LIBRARIES}
)
