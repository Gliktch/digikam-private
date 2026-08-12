#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

add_subdirectory(calibrator)
add_subdirectory(demo)

# -- test the primitive datatypes and helper functions -----------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/primitives_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

# -- test the GeoCoordinates class --------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/geocoordinates_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

# -- test the TileIndex class --------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/tileindex_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

# -- test the LookupAltitudeGeonames class -------------------------------------------------------

# do not add this as a test because it only works if there is an internet connection
#
#ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/lookup_altitude_geonames_utest.cpp
#
#              NAME_PREFIX
#
#              "digikam-"
#
#              LINK_LIBRARIES
#
#              digikamcore
#
#              ${COMMON_TEST_LINK}
#)

# -- test the marker model ------------------------------------------------------------------------

if(NOT WIN32)

    ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/itemmarkertiler_utest.cpp

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  digikamcore

                  ${COMMON_TEST_LINK}
    )

endif()

# -- test the track management classes -------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/tracks_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

# -- track loading application for timing tests ----------------------------------------------------

set(loadtrack_cli_sources ${CMAKE_CURRENT_SOURCE_DIR}/loadtrack_cli.cpp)

add_executable(loadtrack_cli ${loadtrack_cli_sources})

target_link_libraries(loadtrack_cli

                      digikamcore

                      ${COMMON_TEST_LINK}
)
