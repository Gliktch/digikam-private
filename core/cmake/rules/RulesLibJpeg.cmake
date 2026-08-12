#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

DETECT_JPEG()
set(DIGIKAM_LIBJPEG_DIR libjpeg/${JPEG_LIB_VERSION})
message(STATUS "Using libjpeg sub-directory: ${DIGIKAM_LIBJPEG_DIR}")

# JPEG library version check

message(STATUS "Looking for LibJpeg Version...")

if(JPEG_FOUND)

    include_directories(${JPEG_INCLUDE_DIRS})

    # JPEG lib version is done on top level through DETECT_JPEG() from MacroJPEG.cmake

    if (${JPEG_LIB_VERSION} LESS 80)

        message(ERROR "The Libjpeg API is less than 8.0. At least libjpeg >= 8 is required to support DNG compression.")

    else()

        set(JPEG8_FOUND TRUE)

    endif()

endif()
