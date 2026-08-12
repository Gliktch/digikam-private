#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libprogressmanager_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dhistoryview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dprogresswdg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/overlaywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/progressview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/progressmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statusbarprogresswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statusprogressbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/workingwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dworkingpixmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dlogoaction.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_progressmanager_obj OBJECT ${libprogressmanager_SRCS})

target_compile_definitions(core_progressmanager_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
