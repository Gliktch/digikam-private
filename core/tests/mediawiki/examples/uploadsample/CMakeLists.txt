#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(uploadsample_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
)

qt_wrap_ui(uploadsample_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui)

add_executable(uploadsample_gui
               ${uploadsample_SRCS}
)

target_link_libraries(uploadsample_gui

                      PRIVATE

                      digikamcore
                      mediawikibackend

                      ${COMMON_TEST_LINK}
)

