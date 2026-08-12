#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

add_executable(mjpegstream_cli ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstream_cli.cpp)

target_link_libraries(mjpegstream_cli

                      mjpegstreambackend
                      digikamcore

                      ${COMMON_TEST_LINK}
 )
