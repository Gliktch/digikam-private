#
# SPDX-FileCopyrightText: 2026 by François Martin <kde at fmartin dot ch>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>

    ${CMAKE_SOURCE_DIR}/core/app/items/utils
    ${CMAKE_SOURCE_DIR}/core/app/utils
)

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/timelapsefilenamematch_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamgui
              ${COMMON_TEST_LINK}
)
