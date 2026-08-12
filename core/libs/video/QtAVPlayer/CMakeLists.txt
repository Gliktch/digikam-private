#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# QtAVPlayer depencency rules are managed from top level by core/cmake/rules/RulesMediaPlayer.cmake

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    ${DIRECTX_LIBRARY_DIRS}             # Populated under Windows, else empty
)

set(libqtavplayer_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudiocodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudioconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudiofilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudioframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudioinputfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudiooutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudiooutputdevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavaudiooutputfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavcodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavmuxer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavdemuxer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavfiltergraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavfilters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavframecodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavinoutfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qaviodevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavpacket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavplayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavstreamframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavsubtitlecodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavsubtitleframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideobuffer_cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideobuffer_gpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideocodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideofilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideoframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideoinputfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qavvideooutputfilter.cpp
)

if(WIN32)

    set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qavhwdevice_d3d11.cpp
    )

endif()

if(VAAPI_FOUND)

    include_directories(${VAAPI_INCLUDE_DIRS})

    if(VAAPI_DRM_FOUND)
        set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/qavhwdevice_vaapi_drm_egl.cpp
        )
    endif()

    if(VAAPI_X11_FOUND)
        set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/qavhwdevice_vaapi_x11_glx.cpp
        )
    endif()

endif()

if(VDPAU_FOUND)

    include_directories(${VDPAU_INCLUDE_DIRS})

    set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qavhwdevice_vdpau.cpp
    )

endif()

if(APPLE)

    set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qavhwdevice_videotoolbox.mm
    )

endif()

if(HAVE_OPENGL)

    set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qavwidget_opengl.cpp
    )

endif()

if(ANDROID)

    set(libqtavplayer_SRCS ${libqtavplayer_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qavandroidsurfacetexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qavhwdevice_mediacodec.cpp
    )

endif()

# -------------------------------------------------------------------------

# Used by digikamcore
add_library(core_qtavplayer_obj OBJECT ${libqtavplayer_SRCS})

target_compile_definitions(core_qtavplayer_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

target_compile_definitions(core_qtavplayer_obj
                           PRIVATE
                           ${MEDIAPLAYER_DEFINITIONS}
)

target_compile_options(core_qtavplayer_obj
                       PRIVATE
                       ${MEDIAPLAYER_FLAGS}
)
