#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libversionmanager_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/versionmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/versionmanagersettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/versionnamingscheme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/versionfileoperation.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_versionmanager_obj OBJECT ${libversionmanager_SRCS})

target_compile_definitions(core_versionmanager_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
