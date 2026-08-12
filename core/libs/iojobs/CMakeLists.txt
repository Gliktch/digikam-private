#
# SPDX-FileCopyrightText: 2015 by Mohamed_Anwer, <m_dot_anwer at gmx dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libiojobs_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/iojob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iojobdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iojobsthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iojobsmanager.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_iojobs_obj OBJECT ${libiojobs_SRCS})

target_compile_definitions(gui_iojobs_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
