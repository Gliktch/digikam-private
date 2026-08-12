#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

HEADER_DIRECTORIES(${CMAKE_SOURCE_DIR}/core/utilities DK_LOCAL_INCLUDES_UTILS)

include_directories(${DK_LOCAL_INCLUDES}
                    ${DK_LOCAL_INCLUDES_UTILS}
)

add_subdirectory(generic)
add_subdirectory(editor)
add_subdirectory(bqm)
add_subdirectory(rawimport)
add_subdirectory(dimg)
