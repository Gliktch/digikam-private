#
# SPDX-FileCopyrightText: 2019      by Igor Antropov <antropovi at yahoo dot com>
# SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}

                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(wallpaperplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/wallpaperplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wallpaperplugindlg.cpp
)

if(WIN32)

    set(wallpaperplugin_SRCS ${wallpaperplugin_SRCS}
                             ${CMAKE_CURRENT_SOURCE_DIR}/wallpaperplugin_windows.cpp)

    set(compile_wallpaperplugin ON)

elseif(APPLE)

    set(wallpaperplugin_SRCS ${wallpaperplugin_SRCS}
                             ${CMAKE_CURRENT_SOURCE_DIR}/wallpaperplugin_macos.cpp)

    set(compile_wallpaperplugin ON)

elseif(ENABLE_DBUS)

    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)

    set(wallpaperplugin_SRCS ${wallpaperplugin_SRCS}
                             ${CMAKE_CURRENT_SOURCE_DIR}/wallpaperplugin_dbus.cpp)

    set(compile_wallpaperplugin ON)

endif()

if(compile_wallpaperplugin)

    message(STATUS "WallPaper plugin will be compiled......... YES")

    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Wallpaper
                               SOURCES ${wallpaperplugin_SRCS}
    )

else()

    message(STATUS "WallPaper plugin will be compiled......... NO")

endif()
