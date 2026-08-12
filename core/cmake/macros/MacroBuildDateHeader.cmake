# Macro to create build date header
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

macro(BUILD_DATE_HEADER)

    message(STATUS "Configuring build header target...")

    # Add a custom command to drive the build date script

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/builddatescript.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/builddatescript.cmake"
                   @ONLY)

    # Add a custom target to drive the custom command.

    add_custom_target(digikam-builddate ALL COMMAND ${CMAKE_COMMAND} -P
                      "${CMAKE_CURRENT_BINARY_DIR}/builddatescript.cmake")

endmacro()
