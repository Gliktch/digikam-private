#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# Define libjpeg_EXPORTS: kde-win emerged libjpeg uses this define to
# decide whether to make dllimport (by default) or dllexport. So, we need to export.
add_definitions(-Dlibjpeg_EXPORTS)

set(libjpegutils_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/jpegutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iccjpeg.c
    ${DIGIKAM_LIBJPEG_DIR}/transupp.c
)

# no warnings from JPEG extra implementations
if(MSVC)
    set_source_files_properties(${DIGIKAM_LIBJPEG_DIR}/transupp.c PROPERTIES COMPILE_FLAGS /w)
else()
    set_source_files_properties(${DIGIKAM_LIBJPEG_DIR}/transupp.c PROPERTIES COMPILE_FLAGS "-w")
endif()

if(WIN32)
    set(libjpegutils_SRCS
        ${libjpegutils_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/jpegwin.cpp
    )
endif()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_jpegutils_obj OBJECT ${libjpegutils_SRCS})

target_compile_definitions(core_jpegutils_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
