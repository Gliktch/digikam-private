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

set(dimgqimageplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgqimageplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgqimageloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgqimageloader_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgqimageloader_save.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgjxlexportsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgavifexportsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimgwebpexportsettings.cpp
)

DIGIKAM_ADD_DIMG_PLUGIN(NAME    QImage
                        SOURCES ${dimgqimageplugin_SRCS}
)
