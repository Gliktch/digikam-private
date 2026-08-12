#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

###

set(libvideotoolscommon_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/vidslidesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/ffmpegbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/ffmpeglauncher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/vidslidetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/vidslidethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/osd/frameosd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/osd/frameosdsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/osd/frameosdwidget.cpp
)

if(ENABLE_MEDIAPLAYER)

    set(libvideotoolscommon_SRCS
        ${libvideotoolscommon_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/ffmpegconfighelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/player/vidplayerdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/player/audplayerwdg.cpp
    )

endif()

# Used by digikamcore
add_library(core_videotoolscommon_obj OBJECT ${libvideotoolscommon_SRCS})

target_compile_definitions(core_videotoolscommon_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

###

if(ENABLE_MEDIAPLAYER)

    if(Qt6_FOUND AND ENABLE_QTMULTIMEDIA)

        # NOTE: this part depends of QtMultimedia

        set(libvideotools_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/player/qtmm/mediaplayerview.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/player/qtmm/slidevideo.cpp
        )

    else()

        # NOTE: this part depends of QtAVPlayer

        set(libvideotools_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/player/QtAVPlayer/daudioplayer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/player/QtAVPlayer/dvideowidget.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/player/QtAVPlayer/mediaplayerview.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/player/QtAVPlayer/slidevideo.cpp
        )

    endif()

    # Used by digikamcore
    add_library(core_videotools_obj OBJECT ${libvideotools_SRCS})

    target_compile_definitions(core_videotools_obj
                               PRIVATE
                               digikamcore_EXPORTS
    )

    target_compile_definitions(core_videotools_obj
                               PRIVATE
                               ${MEDIAPLAYER_DEFINITIONS}
    )

    # ---

    if (NOT ENABLE_QTMULTIMEDIA)

        add_subdirectory(QtAVPlayer)

    endif()

endif()
