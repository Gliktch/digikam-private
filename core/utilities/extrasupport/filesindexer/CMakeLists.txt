#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(baloowrap_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/baloowrap.cpp)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::FileMetaData,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(core_baloowrap_obj OBJECT ${baloowrap_SRCS})

target_compile_definitions(core_baloowrap_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
