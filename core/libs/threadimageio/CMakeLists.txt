#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libthreadimageio_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/preview/previewloadthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/preview/previewtask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/preview/previewsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/preview/previewosdsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/preview/previewosdwidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailcreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailcreator_basic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailcreator_freedesktop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailcreator_database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailcreator_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailloadthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailloadthread_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailtask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumb/thumbnailsize.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/fileio/loadsavethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileio/loadingdescription.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileio/loadingcache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileio/loadingcacheinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileio/loadsavetask.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/engine/dfileoperations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/filereadwritelock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/managedloadsavethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/sharedloadsavethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/dservicemenu_linux.cpp
)

if(APPLE)
    set(libthreadimageio_SRCS
        ${libthreadimageio_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/dservicemenu_mac.mm
    )
endif()

if(ENABLE_MEDIAPLAYER)

    set(libthreadimageio_SRCS
        ${libthreadimageio_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/video/videostripfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/video/videothumbdecoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/video/videothumbdecoder_p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/video/videothumbwriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/video/videothumbnailer.cpp
    )

endif()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Used by digikamcore
add_library(core_threadimageio_obj OBJECT ${libthreadimageio_SRCS})

target_compile_definitions(core_threadimageio_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
