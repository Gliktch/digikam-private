#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

if(HAVE_MEDIAPLAYER)

    set(QTMEDIA_BACKEND ${MEDIAPLAYER_LIBRARIES})

endif()


include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

set(videoslideshowplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslidewizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslideintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslidealbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslideimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslidevideopage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslideoutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vidslidefinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/videoslideshowplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    VideoSlideShow
                           SOURCES ${videoslideshowplugin_SRCS}
                           DEPENDS ${QTMEDIA_BACKEND}
)
