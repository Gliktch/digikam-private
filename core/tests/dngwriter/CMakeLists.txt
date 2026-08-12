#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(${CMAKE_SOURCE_DIR}/core/cmake/rules/RulesDNGSdk.cmake)

include_directories(${CMAKE_SOURCE_DIR}/core/libs/dngwriter/extra/dng_sdk)

# =======================================================
# dng validate command line tool from DNG SDK

set(dngvalidate_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dngwriter/extra/dng_sdk/dng_validate.cpp)

add_executable(dngvalidate_cli ${dngvalidate_cli_SRCS})
ecm_mark_nongui_executable(dngvalidate_cli)

target_link_libraries(dngvalidate_cli

                      libdng
                      digikamcore

                      ${EXPAT_LIBRARY}
                      ${JXL_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT}
                      PNG::PNG                          # For zlib
                      ${JPEG_LIBRARIES}
                     )

# =======================================================
# dng info command line tool

set(dnginfo_cli_SRCS dnginfo_cli.cpp)

add_executable(dnginfo_cli ${dnginfo_cli_SRCS})
ecm_mark_nongui_executable(dnginfo_cli)

target_link_libraries(dnginfo_cli

                      libdng
                      digikamcore

                      ${COMMON_TEST_LINK}

                      ${EXPAT_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT}
                      PNG::PNG                          # For zlib
                      ${JPEG_LIBRARIES}
                     )

# =======================================================
# dng compare command line tool

set(dngcompare_cli_SRCS dngcompare_cli.cpp)

add_executable(dngcompare_cli ${dngcompare_cli_SRCS})
ecm_mark_nongui_executable(dngcompare_cli)

target_link_libraries(dngcompare_cli

                      libdng
                      digikamcore

                      ${COMMON_TEST_LINK}

                      ${EXPAT_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT}
                      PNG::PNG                          # For zlib
                      ${JPEG_LIBRARIES}
                     )

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/raw2dng_utest.cpp

              GUI

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
              ${EXPAT_LIBRARY}
              ${CMAKE_THREAD_LIBS_INIT}
              ${TIFF_LIBRARIES}
              PNG::PNG                          # For zlib
              ${JPEG_LIBRARIES}
)
