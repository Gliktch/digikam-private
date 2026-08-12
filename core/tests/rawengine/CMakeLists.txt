#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

if(JXL_FOUND)

    set(LIB_DNG libdng)

endif()

include_directories(${CMAKE_SOURCE_DIR}/core/libs/rawengine/libraw
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(raw2png_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/raw2png_cli.cpp)
add_executable(raw2png_cli ${raw2png_cli_SRCS})
target_link_libraries(raw2png_cli

                      digikamcore

                      ${COMMON_TEST_LINK}
)

set(librawinfo_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/librawinfo_cli.cpp)
add_executable(librawinfo_cli ${librawinfo_cli_SRCS})
target_link_libraries(librawinfo_cli

                      digikamcore

                      ${COMMON_TEST_LINK}
)

# -- LibRaw CLI Samples Compilation --------------------------------------------------------------------------------

# A small macro so that this is a bit cleaner

macro(LIBRAW_BUILD_SAMPLES)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target)
    list(GET _temp 1 _ext)

    set(${_target}_SRCS ${CMAKE_SOURCE_DIR}/core/libs/rawengine/libraw/samples/${_filename})

    if (_ext STREQUAL "c")

        while(CMAKE_C_FLAGS MATCHES "-std=iso9899:1990")

            string(REPLACE "-std=iso9899:1990" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

        endwhile()

    endif()

    if(WIN32 AND MSVC)
        set_source_files_properties(${${_target}_SRCS} PROPERTIES COMPILE_FLAGS /w)
    else()
        set_source_files_properties(${${_target}_SRCS} PROPERTIES COMPILE_FLAGS "-w")
    endif()

    add_executable(${_target}_cli ${${_target}_SRCS})

    target_link_libraries(${_target}_cli

                          libraw
                          PNG::PNG                          # For zlib
                          ${LIB_DNG}

                          ${COMMON_TEST_LINK}
    )


    if(WIN32)

        target_compile_definitions(${_target}_cli
                                   PRIVATE
                                   -DWIN32
        )

    endif()


endmacro()

LIBRAW_BUILD_SAMPLES(4channels.cpp)
LIBRAW_BUILD_SAMPLES(dcraw_emu.cpp)
LIBRAW_BUILD_SAMPLES(mem_image_sample.cpp)
LIBRAW_BUILD_SAMPLES(multirender_test.cpp)
LIBRAW_BUILD_SAMPLES(openbayer_sample.cpp)
LIBRAW_BUILD_SAMPLES(postprocessing_benchmark.cpp)
LIBRAW_BUILD_SAMPLES(raw-identify.cpp)
LIBRAW_BUILD_SAMPLES(rawtextdump.cpp)
LIBRAW_BUILD_SAMPLES(simple_dcraw.cpp)
LIBRAW_BUILD_SAMPLES(unprocessed_raw.cpp)

# C sample files which must be compiled only if LCMS v2 is detected, to prevent broken compilation

if(LCMS2_FOUND)

   if(MSVC)

        LIBRAW_BUILD_SAMPLES(half_mt_win32.c)

   else()

        LIBRAW_BUILD_SAMPLES(dcraw_half.c)
        LIBRAW_BUILD_SAMPLES(half_mt.c)

   endif()

endif()
