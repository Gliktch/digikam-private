#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
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

set(onedriveplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/odmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odnewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odtalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    OneDrive
                           SOURCES ${onedriveplugin_SRCS}
)

add_subdirectory(icons)
