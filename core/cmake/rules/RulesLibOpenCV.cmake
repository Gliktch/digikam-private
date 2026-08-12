#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

DETECT_OPENCV(${OPENCV_MIN_VERSION} ${OPENCV_MAX_VERSION} core
                                                          objdetect
                                                          imgproc
                                                          imgcodecs
                                                          dnn
                                                          flann
)

if(${OpenCV_FOUND})

    if(${OpenCV_VERSION} VERSION_LESS ${OPENCV_MIN_VERSION})

        message(STATUS "OpenCV < ${OPENCV_MIN_VERSION} have been found. Please use a more recent version.")
        set(OpenCV_FOUND FALSE)

    endif()

endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app/utils/digikam_opencv.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/app/utils/digikam_opencv.h)

include_directories(${OpenCV_INCLUDE_DIRS})
