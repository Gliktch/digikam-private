#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::NetworkAuth,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/gdrive
                    ${CMAKE_CURRENT_SOURCE_DIR}/gphoto
)

set(googleplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gsnewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gsplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gsreplacedlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gstalkerbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gswindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gdrive/gdmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gdrive/gdtalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gphoto/gpmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gphoto/gptalker.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Google
                           SOURCES ${googleplugin_SRCS}
)

add_subdirectory(icons)
