#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(Doxygen)

PRINT_OPTIONAL_LIBRARY_STATUS("doxygen" "https://github.com/doxygen/doxygen" "(version >= 1.8.0)" "digiKam will be configured without API documentation building support." Doxygen_FOUND)

if(Doxygen_FOUND)

    message(STATUS "Doxygen is found.......................... ${Doxygen_FOUND}")
    message(STATUS "Graphviz Dot is found..................... ${DOXYGEN_DOT_FOUND}")

    if(Doxygen_FOUND AND DOXYGEN_DOT_FOUND)

        message(STATUS "To build API documentation use 'make doc'")

        add_custom_target(doc ${DOXYGEN_EXECUTABLE}
                          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    endif()

endif()
