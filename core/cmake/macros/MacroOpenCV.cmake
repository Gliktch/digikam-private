# A macro wrapper to find OpenCV library
#
# Syntax:  DETECT_OPENCV(OPENCV_MIN_VERSION OPENCV_REQUIRED_COMPONENTS)
#
# Example: DETECT_OPENCV(3.3.0 4.8.0 core highgui objdetect contrib)
# which try to find OpenCV version between 3.3.0 and 4.8.0
# with internal components "core", "highgui", "objdetect", and "contrib".
#
# Once done this will be adjusted:
#
# OpenCV_FOUND        - OpenCV detection flag
# OpenCV_INCLUDE_DIRS - OpenCV include directories
# OpenCV_LIBS         - OpenCV libraries
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

macro(DETECT_OPENCV OPENCV_MIN_VERSION OPENCV_MAX_VERSION)

    # Reset to avoid picking up extra libraries
    set(OpenCV_LIBS)

    set(OPENCV_REQUIRED_COMPONENTS "${ARGN}")

    find_package(OpenCV COMPONENTS ${OPENCV_REQUIRED_COMPONENTS} PATHS /usr/local)

    if(OpenCV_FOUND)

        message(STATUS "OpenCV Root directory is: ${OpenCV_DIR}")

        # check OpenCV version

        if(OpenCV_VERSION)

            message(STATUS "OpenCV: Found version ${OpenCV_VERSION} (required min|max: ${OPENCV_MIN_VERSION}|${OPENCV_MAX_VERSION})")

            if(${OpenCV_VERSION} VERSION_LESS ${OPENCV_MIN_VERSION})

                message(WARNING "OpenCV: Version is too old.")
                set(OpenCV_FOUND FALSE)

            endif()

            if(${OpenCV_VERSION} VERSION_GREATER ${OPENCV_MAX_VERSION})

                message(WARNING "OpenCV: Version is not yet supported.")
                set(OpenCV_FOUND FALSE)

            endif()

        else()

            message(WARNING "OpenCV: Version information not found, your version is probably too old.")
            set(OpenCV_FOUND FALSE)

        endif()

        message(STATUS "OpenCV headers: ${OpenCV_INCLUDE_DIRS}")
        message(STATUS "OpenCV libs   : ${OpenCV_LIBS}")

    endif()

endmacro()
