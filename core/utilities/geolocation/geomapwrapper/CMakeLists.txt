#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libgeomapwrapper_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/gpsiteminfo.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/gpsiteminfosorter.cpp)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_geomapwrapper_obj OBJECT ${libgeomapwrapper_SRCS})

target_compile_definitions(core_geomapwrapper_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
