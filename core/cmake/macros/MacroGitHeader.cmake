# Macro to create git version header
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

macro(GIT_HEADER)

    message(STATUS "Configuring git header target...")

    # We only do this IF we are in a .git dir

    find_file(GIT_MARKER entries PATHS ${CMAKE_SOURCE_DIR}/.git)

    if(NOT GIT_MARKER)

        set(GIT_MARKER ${CMAKE_SOURCE_DIR}/CMakeLists.txt)  # Dummy file

    endif()

    # Add a custom command to drive the git script whenever the git entries
    # file changes.

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gitscript.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake"
                   @ONLY)

    # Add a custom target to drive the custom command.

    add_custom_target(digikam-gitversion ALL COMMAND ${CMAKE_COMMAND} -P
                      "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake")

endmacro()
