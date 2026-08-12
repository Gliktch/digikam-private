#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2024      by Michael Miller, <michael underscore miller at msn dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

kde_enable_exceptions()

set(libqtopencvimg_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/qtopencvimg.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_qtopencvimg_obj OBJECT ${libqtopencvimg_SRCS})

target_compile_definitions(core_qtopencvimg_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
