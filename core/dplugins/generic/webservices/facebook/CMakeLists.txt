#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(facebookplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/fbmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbnewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbtalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbtalker_wizard.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    FaceBook
                           SOURCES ${facebookplugin_SRCS}
)

add_subdirectory(icons)
