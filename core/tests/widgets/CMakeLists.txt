#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
)

##################################################################

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/searchtextbar_utest.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/thememanager_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

##################################################################

add_executable(ditemslist_gui ${CMAKE_CURRENT_SOURCE_DIR}/ditemslist_gui.cpp)

target_link_libraries(ditemslist_gui

                      digikamcore

                      ${COMMON_TEST_LINK}
)

##################################################################

add_executable(dpreviewmanager_gui ${CMAKE_CURRENT_SOURCE_DIR}/dpreviewmanager_gui.cpp)

target_link_libraries(dpreviewmanager_gui

                      digikamcore

                      ${COMMON_TEST_LINK}
)
