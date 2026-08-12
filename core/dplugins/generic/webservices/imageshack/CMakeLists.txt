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

set(imageshackplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshackmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshackplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshacktalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshackwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshacknewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshacksession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageshackwidget.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    ImageShack
                           SOURCES ${imageshackplugin_SRCS}
)

add_subdirectory(icons)
