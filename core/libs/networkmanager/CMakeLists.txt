#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libnetworkmanager_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/networkmanager.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_networkmanager_obj OBJECT ${libnetworkmanager_SRCS})

target_compile_definitions(core_networkmanager_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
