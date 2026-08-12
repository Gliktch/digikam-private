#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# Boost uses operator names (and, not, ...)
string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories(
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
)

#------------------------------------------------------------------------

set(equalizefilter_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/equalizefilter_cli.cpp)
add_executable(equalizefilter_cli ${equalizefilter_cli_SRCS})
ecm_mark_nongui_executable(equalizefilter_cli)

target_link_libraries(equalizefilter_cli

                      digikamcore

                      ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

set(colorbalancefilter_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/colorbalancefilter_cli.cpp)
add_executable(colorbalancefilter_cli ${colorbalancefilter_cli_SRCS})
ecm_mark_nongui_executable(colorbalancefilter_cli)

target_link_libraries(colorbalancefilter_cli

                      digikamcore

                      ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

if(ImageMagick_Magick++_FOUND)

    set(magickloader_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/magickloader_cli.cpp)
    add_executable(magickloader_cli ${magickloader_cli_SRCS})

    target_link_libraries(magickloader_cli

                          digikamcore

                          ${COMMON_TEST_LINK}
    )

endif()

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/dimgfilteraction_utest.cpp

              GUI

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/dimgfilteractionkeys_utest.cpp

        GUI

        NAME_PREFIX

        "digikam-"

        LINK_LIBRARIES

        digikamcore

        ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/dimgloader_utest.cpp

        GUI

        NAME_PREFIX

        "digikam-"

        LINK_LIBRARIES

        digikamcore

        ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/dimgfreerotation_utest.cpp

              GUI

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

add_library(libabstracthistory STATIC ${CMAKE_CURRENT_SOURCE_DIR}/dimgabstracthistory_utest.cpp)

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/dimghistory_utest.cpp

              GUI

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              libabstracthistory

              digikamcore

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

# TODO: This unit-test do not link yet under Microsoft/Apple compilers about HistoryGraphData

if(NOT MSVC AND NOT APPLE)

    ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/dimghistorygraph_utest.cpp

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  digikamcore
                  digikamdatabase
                  libmodeltest
                  libabstracthistory

                  ${COMMON_TEST_LINK}
    )

endif()
