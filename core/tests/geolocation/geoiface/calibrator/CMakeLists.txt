#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
)

set(calibrator_sources_gui ${CMAKE_CURRENT_SOURCE_DIR}/calibrator.cpp)

add_executable(geoiface_calibrator_gui ${calibrator_sources_gui})

target_link_libraries(geoiface_calibrator_gui

                      digikamcore

                      ${COMMON_TEST_LINK}
)
