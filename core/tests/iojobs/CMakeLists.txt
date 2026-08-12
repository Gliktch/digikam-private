#
# SPDX-FileCopyrightText: 2015,      Mohamed_Anwer, <m_dot_anwer at gmx dot com>
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_executable(IOJobsTest ${CMAKE_CURRENT_SOURCE_DIR}/iojobs_utest.cpp)

ecm_mark_nongui_executable(IOJobsTest)

target_link_libraries(IOJobsTest

                      digikamgui

                      ${COMMON_TEST_LINK}
)
