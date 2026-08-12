#
# CMake script to find FFMPEG libraries
# Once done this will define:
#
#  FFMPEG_FOUND       - system has ffmpeg
#  FFMPEG_INCLUDE_DIR - Include directory necessary for using the ffmpeg headers
#  FFMPEG_LIBRARIES   - Link these to use ffmpeg
#  FFMPEG_DEFINITIONS - Compiler switches required for using ffmpeg
#
# For each of the components it will additionally set.
#
#   - AVCODEC
#   - AVDEVICE
#   - AVFORMAT
#   - AVUTIL
#   - POSTPROCESS
#   - SWSCALE
#   - AVRESAMPLE
#   - SWRESAMPLE
#
# NOTE: default components are AVFORMAT, AVUTIL, AVCODEC.
#
# The following variables will be defined
#
#   <component>_FOUND        - System has <component>
#   <component>_INCLUDE_DIRS - Include directory necessary for using the <component> headers
#   <component>_LIBRARIES    - Link these to use <component>
#   <component>_DEFINITIONS  - Compiler switches required for using <component>
#   <component>_VERSION      - The components version
#
# SPDX-FileCopyrightText: 2006,     by Matthias Kretz, <kretz at kde dot org>
# SPDX-FileCopyrightText: 2008,     by Alexander Neundorf, <neundorf at kde dot org>
# SPDX-FileCopyrightText: 2011      by Michael Jansen, <kde at michael-jansen dot biz>
# SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

INCLUDE(FindPackageHandleStandardArgs)

# The default components were taken from a survey over other FindFFMPEG.cmake files

IF(NOT FFmpeg_FIND_COMPONENTS)

    SET(FFmpeg_FIND_COMPONENTS AVCODEC AVFILTER AVFORMAT AVUTIL SWSCALE)

ENDIF()

# Macro to marks the given component as found if both *_LIBRARIES AND *_INCLUDE_DIRS is present.
#
MACRO(set_component_found _component)

    IF(${_component}_LIBRARIES AND ${_component}_INCLUDE_DIRS)

        # message(STATUS "  - ${_component} found.")

        SET(${_component}_FOUND TRUE)

    ELSE()

        # message(STATUS "  - ${_component} not found.")

        SET(${_component}_FOUND FALSE)

    ENDIF()

ENDMACRO()

# Macro to checks for the given component by invoking pkgconfig and then looking up the libraries and
# include directories.

MACRO(find_component _component _pkgconfig _library _header)

    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls

    FIND_PACKAGE(PkgConfig)

    IF(PKG_CONFIG_FOUND)

        MESSAGE(STATUS "Use PkgConfig to check FFMPEG ${_component} availability...")
        pkg_check_modules(PC_${_component} ${_pkgconfig})

    ELSE()

        MESSAGE(WARNING "PkgConfig not found!")

    ENDIF()

    FIND_PATH(${_component}_INCLUDE_DIRS ${_header}
              HINTS
                  ${PC_${_component}_INCLUDEDIR}
                  ${PC_${_component}_INCLUDE_DIRS}
              PATH_SUFFIXES
                  ffmpeg
    )

    FIND_LIBRARY(${_component}_LIBRARIES NAMES ${_library}
                 HINTS
                     ${PC_${_component}_LIBDIR}
                     ${PC_${_component}_LIBRARY_DIRS}
    )

    SET(${_component}_DEFINITIONS  ${PC_${_component}_CFLAGS_OTHER}
        CACHE STRING "The ${_component} CFLAGS.")

    SET(${_component}_VERSION      ${PC_${_component}_VERSION}
        CACHE STRING "The ${_component} version number.")

    set_component_found(${_component})

    MARK_AS_ADVANCED(
        ${_component}_INCLUDE_DIRS
        ${_component}_LIBRARIES
        ${_component}_DEFINITIONS
        ${_component}_VERSION
    )

ENDMACRO()

# Check for cached results. If there are skip the costly part.

if(NOT FFMPEG_LIBRARIES)

    # Check for all possible component.

    find_component(AVCODEC    libavcodec    avcodec    libavcodec/avcodec.h)
    find_component(AVFILTER   libavfilter   avfilter   libavfilter/avfilter.h)
    find_component(AVFORMAT   libavformat   avformat   libavformat/avformat.h)
    find_component(AVDEVICE   libavdevice   avdevice   libavdevice/avdevice.h)
    find_component(AVUTIL     libavutil     avutil     libavutil/avutil.h)
    find_component(SWSCALE    libswscale    swscale    libswscale/swscale.h)
    find_component(POSTPROC   libpostproc   postproc   libpostproc/postprocess.h)
    find_component(AVRESAMPLE libavresample avresample libavresample/avresample.h)
    find_component(SWRESAMPLE libswresample swresample libswresample/swresample.h)

    # Check if the required components were found and add their stuff to the FFMPEG_* vars.

    FOREACH(_component ${FFmpeg_FIND_COMPONENTS})

        IF(${_component}_FOUND)

            # message(STATUS "Required component ${_component} present.")

            SET(FFMPEG_LIBRARIES   ${FFMPEG_LIBRARIES}   ${${_component}_LIBRARIES})
            SET(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${${_component}_DEFINITIONS})
            LIST(APPEND FFMPEG_INCLUDE_DIRS ${${_component}_INCLUDE_DIRS})

        ELSE()

            # message(STATUS "Required component ${_component} missing.")

        ENDIF()

    ENDFOREACH()

    # Build the include path with duplicates removed.

    IF(FFMPEG_INCLUDE_DIRS)

        LIST(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)

    ENDIF()

    # cache the vars.

    SET(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} CACHE STRING "The FFmpeg include directories." FORCE)
    SET(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    CACHE STRING "The FFmpeg libraries."           FORCE)
    SET(FFMPEG_DEFINITIONS  ${FFMPEG_DEFINITIONS}  CACHE STRING "The FFmpeg cflags."              FORCE)

    MARK_AS_ADVANCED(FFMPEG_INCLUDE_DIRS
                     FFMPEG_LIBRARIES
                     FFMPEG_DEFINITIONS
    )

ENDIF()

# Now set the non-cached _FOUND vars for the components.

FOREACH(_component AVCODEC AVDEVICE AVFILTER AVFORMAT AVUTIL POSTPROCESS SWSCALE AVRESAMPLE SWRESAMPLE)

    set_component_found(${_component})

ENDFOREACH()

# Compile the list of required vars
SET(_FFmpeg_REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS)

FOREACH(_component ${FFmpeg_FIND_COMPONENTS})

    LIST(APPEND _FFmpeg_REQUIRED_VARS ${_component}_LIBRARIES ${_component}_INCLUDE_DIRS)

ENDFOREACH()

# Give a nice error message if some of the required vars are missing.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFmpeg DEFAULT_MSG ${_FFmpeg_REQUIRED_VARS})

MESSAGE(STATUS "FFMPEG_FOUND        = ${FFMPEG_FOUND}")
MESSAGE(STATUS "FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}")
MESSAGE(STATUS "FFMPEG_LIBRARIES    = ${FFMPEG_LIBRARIES}")
MESSAGE(STATUS "FFMPEG_DEFINITIONS  = ${FFMPEG_DEFINITIONS}")
