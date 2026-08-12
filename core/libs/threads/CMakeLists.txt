#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libdthread_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/actionjob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actionthreadbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threadmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/workerobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamicthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parallelworkers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processlauncher.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_dthread_obj OBJECT ${libdthread_SRCS})

target_compile_definitions(core_dthread_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
