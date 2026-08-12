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
#                    $<TARGET_PROPERTY:Qt5::XmlPatterns,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(rajceplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcealbum.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcempform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajceplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcetalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcewindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcecommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcenewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcesession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rajcewidget.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Rajce
                           SOURCES ${rajceplugin_SRCS}
                           DEPENDS Qt5::XmlPatterns
)

add_subdirectory(icons)
