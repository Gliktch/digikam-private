#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Increase the stack size to prevent crashes with GMic-qt.

if (APPLE)

    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

        message(STATUS "Increase GCC linker stack size to 16MB under MacOS")

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,16777216")
#        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--stack,16777216")
#        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--stack,16777216")

    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

        message(STATUS "Increase Clang linker stack size to 16MB under MacOS")

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000")
#        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000")
#       set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000")

    endif()

endif()
