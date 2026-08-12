#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

HEADER_DIRECTORIES(${CMAKE_SOURCE_DIR}/core/utilities DK_LOCAL_INCLUDES_UTILS)

include_directories(${DK_LOCAL_INCLUDES}
                    ${DK_LOCAL_INCLUDES_UTILS}

                    ${CMAKE_SOURCE_DIR}/core/app/main
                    ${CMAKE_SOURCE_DIR}/core/app/date
                    ${CMAKE_SOURCE_DIR}/core/app/dragdrop
                    ${CMAKE_SOURCE_DIR}/core/app/views/stack
                    ${CMAKE_SOURCE_DIR}/core/app/views/utils
                    ${CMAKE_SOURCE_DIR}/core/app/views/sidebar
                    ${CMAKE_SOURCE_DIR}/core/app/items/utils
                    ${CMAKE_SOURCE_DIR}/core/app/items/overlays
                    ${CMAKE_SOURCE_DIR}/core/app/items/views
                    ${CMAKE_SOURCE_DIR}/core/app/items/thumbbar
                    ${CMAKE_SOURCE_DIR}/core/app/views/preview
)

add_subdirectory(advancedrename)
add_subdirectory(queuemanager)
add_subdirectory(imageeditor)
add_subdirectory(lighttable)
add_subdirectory(survey)
add_subdirectory(firstrun)
add_subdirectory(import)
add_subdirectory(setup)
add_subdirectory(fuzzysearch)
add_subdirectory(maintenance)
add_subdirectory(searchwindow)
add_subdirectory(extrasupport)
add_subdirectory(facemanagement)
add_subdirectory(focuspointmanagement)

if(${ENABLE_GEOLOCATION})

    add_subdirectory(geolocation)

endif()
