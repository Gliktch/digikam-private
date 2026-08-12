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

# --------------------------------------------------------------

macro(IMGQSORT_BUILD_STANDALONE)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target)

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_filename}
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgqsorttest_shared.cpp)

    add_executable(${_target} ${${_target}_SRCS})

    target_link_libraries(${_target}

                          digikamcore
                          digikamgui

                          ${COMMON_TEST_LINK}
    )

endmacro()


IMGQSORT_BUILD_STANDALONE(detectblur_cli.cpp)
IMGQSORT_BUILD_STANDALONE(detectnoise_cli.cpp)
IMGQSORT_BUILD_STANDALONE(detectexposure_cli.cpp)
IMGQSORT_BUILD_STANDALONE(detectcompression_cli.cpp)
IMGQSORT_BUILD_STANDALONE(detectgeneral_cli.cpp)

# --------------------------------------------------------------

if (Qt6_FOUND)
    QT6_WRAP_CPP(MOC_files ${CMAKE_CURRENT_SOURCE_DIR}/imgqsorttest.h)
else()
    QT5_WRAP_CPP(MOC_files ${CMAKE_CURRENT_SOURCE_DIR}/imgqsorttest.h)
endif()

add_library(libimgqsorttest STATIC imgqsorttest_shared.cpp ${MOC_files})
target_link_libraries(libimgqsorttest ${COMMON_TEST_LINK})

macro(IMGQSORT_BUILD_UNITTEST)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target)

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_filename})

    ecm_add_tests(${${_target}_SRCS}

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  libimgqsorttest

                  digikamcore
                  digikamgui

                  ${COMMON_TEST_LINK}
    )

endmacro()

# IMGQSORT_BUILD_UNITTEST(detectgeneral_badimage_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectgeneral_goodimage_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectgeneral_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectnoise_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectblur_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectexposure_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectcompression_utest.cpp)
# IMGQSORT_BUILD_UNITTEST(detectaesthetic_utest.cpp)

# --------------------------------------------------------------

# set(_filename NIMA_inference_cli.cpp)
# string(REPLACE "." ";" _temp ${_filename})
# list(GET _temp 0 _target)

# set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_filename})

# add_executable(${_target} ${${_target}_SRCS})

# target_link_libraries(${_target}
#                         digikamcore
#                         digikamgui
#                         ${COMMON_TEST_LINK}
# )
