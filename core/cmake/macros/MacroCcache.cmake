# Macro to speed-up compilations using ccache.
# https://ccache.dev/
#
# Check ccache program availability and enable compiler rules accordingly.
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

MACRO(MACRO_COMPILER_CCACHE)

    IF(NOT MSVC)

        MESSAGE(STATUS "Looking for Ccache...")

        FIND_PROGRAM(CCACHE_FOUND ccache)

        MESSAGE(STATUS "Ccache program found: ${CCACHE_FOUND}...")

        # to use ccache in compile

        IF(CCACHE_FOUND)

            # Get the full text including Ccache version number.

            EXECUTE_PROCESS(COMMAND ccache --version
                            OUTPUT_VARIABLE CCACHE_VERSION_OUTPUT
                            ERROR_QUIET
                            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            # First line extraction

            STRING(REGEX REPLACE "\n.*" "" CCACHE_VERSION_LINE "${CCACHE_VERSION_OUTPUT}")

            STRING(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+)?" CCACHE_VERSION "${CCACHE_VERSION_LINE}")

            MESSAGE(STATUS "Ccache version: ${CCACHE_VERSION}")
            MESSAGE(STATUS "Using Ccache to speed-up compilations...")
            SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
            SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK    ccache)

        ELSE()

            MESSAGE(WARNING "ccache cannot be used to speed-up compilations...")

        ENDIF()

    ENDIF()

ENDMACRO()
