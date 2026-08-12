#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

message(STATUS "")
message(STATUS "--------------------------------------------------")
message(STATUS "digiKam RawEngine dependencies checks:")
message(STATUS "")

include(MacroOpenMP)
DETECT_OPENMP()

if(OPENMP_FOUND)

    message(STATUS "RawEngine will be compiled with OpenMP support : yes")

else()

    # To force libraw to use OpenMP with Macports and MXE.

    if (APPLE OR MINGW)

        add_definitions(-DLIBRAW_FORCE_OPENMP)
        set(OPENMP_FOUND ON)

        message(STATUS "RawEngine will be compiled with OpenMP support : yes")

    else()

        message(STATUS "RawEngine will be compiled with OpenMP support : no")

    endif()

endif()

# Flag to use libjasper with LibRaw RedCine codec

if(Jasper_FOUND)

    add_definitions(-DUSE_JASPER)
    include_directories(${JASPER_INCLUDE_DIR})
    message(STATUS "RawEngine will be compiled with RedCine codec  : yes")

else()

    message(STATUS "RawEngine will be compiled with RedCine codec  : no")

endif()

# Flag to use libjpeg with LibRaw DNG lossy codec

if(JPEG8_FOUND)

    add_definitions(-DUSE_JPEG)
    add_definitions(-DUSE_JPEG8)
    message(STATUS "RawEngine will be compiled with DNG lossy codec: yes")

else()

    message(STATUS "RawEngine will be compiled with DNG lossy codec: no")

endif()

set(PTHREADS_FOUND (CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT))

if(PTHREADS_FOUND)

    message(STATUS "RawEngine will be compiled with pthread        : yes")

else()

    message(STATUS "RawEngine will be compiled with pthread        : no")

endif()

# Registration of Libraw configuration to a dedicated header

MACRO_BOOL_TO_01(JPEG8_FOUND  LIBRAW_USE_DNGLOSSYCODEC)
MACRO_BOOL_TO_01(OPENMP_FOUND LIBRAW_USE_OPENMP)
MACRO_BOOL_TO_01(Jasper_FOUND LIBRAW_USE_REDCINECODEC)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libs/rawengine/libraw/libraw_config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/libs/rawengine/libraw/libraw_config.h)
