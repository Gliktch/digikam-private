#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2011-2015, Benjamin Girault, <benjamin dot girault at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/panorama/panoparser/ptoparser
                    ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/panorama/panoparser/ptotype
)

add_executable(panotestparser_cli ${CMAKE_CURRENT_SOURCE_DIR}/panotestparser_cli.cpp)

target_link_libraries(panotestparser_cli

                      digikamcore
                      panoparser

                      ${COMMON_TEST_LINK}
)
