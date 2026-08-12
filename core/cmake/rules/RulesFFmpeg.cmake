#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(FFmpeg REQUIRED COMPONENTS AVCODEC
                                        AVDEVICE
                                        AVFILTER
                                        AVFORMAT
                                        AVUTIL
                                        SWSCALE
                                        SWRESAMPLE
)

find_package(FFmpeg OPTIONAL_COMPONENTS AVRESAMPLE)     # removed with ffmpeg 5

message(STATUS "FFMpeg AVCodec    (required) : ${AVCODEC_FOUND} (${AVCODEC_VERSION})")
message(STATUS "FFMpeg AVDevice   (required) : ${AVDEVICE_FOUND} (${AVDEVICE_VERSION})")
message(STATUS "FFMpeg AVFilter   (required) : ${AVFILTER_FOUND} (${AVFILTER_VERSION})")
message(STATUS "FFMpeg AVFormat   (required) : ${AVFORMAT_FOUND} (${AVFORMAT_VERSION})")
message(STATUS "FFMpeg AVUtil     (required) : ${AVUTIL_FOUND} (${AVUTIL_VERSION})")
message(STATUS "FFMpeg SWScale    (required) : ${SWSCALE_FOUND} (${SWSCALE_VERSION})")
message(STATUS "FFMpeg SWResample (required) : ${SWRESAMPLE_FOUND} (${SWRESAMPLE_VERSION})")
message(STATUS "FFMpeg AVResample (optional) : ${AVRESAMPLE_FOUND} (${AVRESAMPLE_VERSION})")

if(${AVCODEC_FOUND}     AND
   ${AVDEVICE_FOUND}    AND
   ${AVFILTER_FOUND}    AND
   ${AVFORMAT_FOUND}    AND
   ${AVUTIL_FOUND}      AND
   ${SWSCALE_FOUND}     AND
   ${SWRESAMPLE_FOUND}
  )

    include_directories(${FFMPEG_INCLUDE_DIRS})
    set(FFMPEG_FOUND ON)
    message(STATUS "FFmpeg support is enabled : yes")

else()

    set(ENABLE_MEDIAPLAYER OFF)
    set(FFMPEG_FOUND OFF)
    message(STATUS "FFmpeg support is enabled : no")

endif()

if (${FFMPEG_FOUND})

    # Check if FFMPEG 5 API is available

    if (AVCODEC_VERSION)

        string(REPLACE "." ";" VERSION_LIST ${AVCODEC_VERSION})
        list(GET VERSION_LIST 0 AVCODEC_VERSION_MAJOR)
        list(GET VERSION_LIST 1 AVCODEC_VERSION_MINOR)
        list(GET VERSION_LIST 2 AVCODEC_VERSION_PATCH)

        if (${AVCODEC_VERSION_MAJOR} GREATER_EQUAL 59)

            set(FFMPEG_VER5_FOUND 1)

        endif()

    endif()

    if (FFMPEG_VER5_FOUND)

        message(STATUS "FFMpeg >= 5 API           : yes")

        # This definition is used in video thumbnails code.

        add_definitions(-DHAVE_FFMPEG_VERSION5)

    else()

        message(STATUS "FFMpeg >= 5 API           : no")

    endif()

    MACRO_BOOL_TO_01(AVCODEC_FOUND         HAVE_LIBAVCODEC)
    MACRO_BOOL_TO_01(AVDEVICE_FOUND        HAVE_LIBAVDEVICE)
    MACRO_BOOL_TO_01(AVFILTER_FOUND        HAVE_LIBAVFILTER)
    MACRO_BOOL_TO_01(AVFORMAT_FOUND        HAVE_LIBAVFORMAT)
    MACRO_BOOL_TO_01(AVUTIL_FOUND          HAVE_LIBAVUTIL)
    MACRO_BOOL_TO_01(SWSCALE_FOUND         HAVE_LIBSWSCALE)
    MACRO_BOOL_TO_01(SWRESAMPLE_FOUND      HAVE_LIBSWRESAMPLE)
    MACRO_BOOL_TO_01(AVRESAMPLE_FOUND      HAVE_LIBAVRESAMPLE)

    if(NOT AVRESAMPLE_FOUND)

        set(AVRESAMPLE_LIBRARIES "")

    endif()

    set(MEDIAPLAYER_LIBRARIES ${AVCODEC_LIBRARIES}
                              ${AVDEVICE_LIBRARIES}
                              ${AVFILTER_LIBRARIES}
                              ${AVFORMAT_LIBRARIES}
                              ${AVUTIL_LIBRARIES}
                              ${SWSCALE_LIBRARIES}
                              ${SWRESAMPLE_LIBRARIES}
                              ${AVRESAMPLE_LIBRARIES}   # optional with FFMPEG 4 and removed with FFMPEG 5
                              ${CMAKE_DL_LIBS}
    )

endif()
