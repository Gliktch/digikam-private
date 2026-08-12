#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# NOTE: include this common rules on top level for DNGWriter and RawEngine.

include(RulesDNGSdk)

HEADER_DIRECTORIES(${CMAKE_SOURCE_DIR}/core/utilities DK_LOCAL_INCLUDES_UTILS)

include_directories(${DK_LOCAL_INCLUDES}
                    ${DK_LOCAL_INCLUDES_UTILS}

                    ${CMAKE_SOURCE_DIR}/core/app/main
                    ${CMAKE_SOURCE_DIR}/core/app/date
                    ${CMAKE_SOURCE_DIR}/core/app/dragdrop
                    ${CMAKE_SOURCE_DIR}/core/app/views/stack
                    ${CMAKE_SOURCE_DIR}/core/app/views/utils
                    ${CMAKE_SOURCE_DIR}/core/app/items/utils
                    ${CMAKE_SOURCE_DIR}/core/app/items/delegate
                    ${CMAKE_SOURCE_DIR}/core/app/views/sidebar
)

add_subdirectory(dimg)
add_subdirectory(dragdrop)
add_subdirectory(metadataengine)
add_subdirectory(database)
add_subdirectory(dtrash)
add_subdirectory(facesengine)
add_subdirectory(iojobs)
add_subdirectory(imgqsort)
add_subdirectory(jpegutils)
add_subdirectory(pgfutils)
add_subdirectory(threadimageio)
add_subdirectory(widgets)
add_subdirectory(properties)
add_subdirectory(progressmanager)
add_subdirectory(threads)
add_subdirectory(versionmanager)
add_subdirectory(networkmanager)
add_subdirectory(notificationmanager)
add_subdirectory(models)
add_subdirectory(onlineversion)
add_subdirectory(template)
add_subdirectory(dialogs)
add_subdirectory(dplugins)
add_subdirectory(fileactionmanager)
add_subdirectory(filters)
add_subdirectory(settings)
add_subdirectory(rawengine)
add_subdirectory(album)
add_subdirectory(tags)
add_subdirectory(transitionmngr)
add_subdirectory(timeadjust)
add_subdirectory(video)
add_subdirectory(dnnmodelmanager)
add_subdirectory(mlfoundation)
add_subdirectory(qtopencvimg)
add_subdirectory(autorotator)
add_subdirectory(aitoolspipeline)

if(JXL_FOUND)

    add_subdirectory(dngwriter)

endif()
