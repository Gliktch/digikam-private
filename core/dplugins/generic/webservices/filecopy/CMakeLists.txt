#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann, <metzpinguin at gmail dot com>
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

set(filecopyplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/fcexportwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcexportwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fctask.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    FileCopy
                           SOURCES ${filecopyplugin_SRCS}
)
