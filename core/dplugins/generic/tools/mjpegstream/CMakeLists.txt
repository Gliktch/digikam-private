#
# SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libmjpegstream_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstreamdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstreamdlg_settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstreamdlg_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstreamdlg_views.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegframethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegframetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegserver_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegservermngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstreamsettings.cpp
)

# For Unit-tests and plugin
add_library(mjpegstreambackend STATIC ${libmjpegstream_SRCS})

if(MSVC)

    target_compile_options(mjpegstreambackend PRIVATE /W0)

else()

    target_compile_options(mjpegstreambackend PRIVATE -w)

endif()

target_link_libraries(mjpegstreambackend
                      PRIVATE
                      digikamcore
)

if(WIN32)

    target_link_libraries(mjpegstreambackend
                          PRIVATE
                          # Defined in RulesWindows.cmake
                          ${WSOCK32_LIBRARY}
                          ${WS2_32_LIBRARY}
    )

endif()

# ---------------------------------------------------------------------------------------------------

set(mjpegstreamplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mjpegstreamplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    MjpegStream
                           SOURCES ${mjpegstreamplugin_SRCS}
                           DEPENDS mjpegstreambackend
)
