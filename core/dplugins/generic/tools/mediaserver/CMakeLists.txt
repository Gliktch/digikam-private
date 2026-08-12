#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2017      by Ahmed Fathi, <ahmed dot fathi dot abdelmageed at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

HEADER_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk DK_LOCAL_INCLUDES_UPNPSDK)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/server
                    ${DK_LOCAL_INCLUDES_UPNPSDK}
)

# ---------------------------------------------------------------------------------------------------

set(upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltAction.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltArgument.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltConstants.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltCtrlPoint.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltCtrlPointTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltDatagramStream.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltDeviceData.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltDeviceHost.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltEvent.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttpClientTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttpServer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttpServerTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltIconsData.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltMimeType.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltProtocolInfo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltService.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltSsdp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltStateVariable.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltTaskManager.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltThreadTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltUPnP.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltDidl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltFileMediaServer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaBrowser.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaCache.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaItem.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaServer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/ConnectionManagerSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/ContentDirectorySCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/ContentDirectorywSearchSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltSyncMediaBrowser.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/PltMediaRenderer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/PltMediaController.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/AVTransportSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/RdrConnectionManagerSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/RenderingControlSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaConnect/X_MS_MediaReceiverRegistrarSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaConnect/PltMediaConnect.cpp

                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptAutomaticCleaner.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptBase64.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptBufferedStreams.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptCommon.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptDataBuffer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptFile.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptHash.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptHttp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptList.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptLogging.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptMessaging.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptNetwork.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptQueue.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptResults.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptRingBuffer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptSimpleMessageQueue.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptSockets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptStreams.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptStrings.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptSystem.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptThreads.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptTime.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptTls.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptUri.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptUtils.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptXml.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Bsd/NptBsdSockets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Bsd/NptBsdResolver.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/StdC/NptStdcDebug.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/StdC/NptStdcEnvironment.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/StdC/NptStdcFile.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixFile.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixEnvironment.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Null/NptNullAutoreleasePool.cpp
)

if(NOT MSVC)

    # Always compiled excepted under Windows with MSVC compiler.

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixSystem.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptSelectableMessageQueue.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixQueue.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixThreads.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptDebug.cpp
    )

endif()

if(WIN32)

    # Under Windows with MXE

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Time.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32File.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Network.cpp
    )

else()

    # For Linux and Apple

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixTime.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Bsd/NptBsdNetwork.cpp
    )

endif()

if(APPLE)

    # For Apple only

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Apple/NptAppleAutoreleasePool.mm
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Apple/NptAppleLogConfig.mm
    )

endif()

if(MSVC)

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32DynamicLibraries.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32MessageQueue.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Queue.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32SerialPort.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32System.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Threads.cpp
    )

endif()

# Don't process automoc on UpnpSdk headers

file(GLOB_RECURSE upnpsdk_headers ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/*.h)

foreach(_file ${upnpsdk_headers})

    set_property(SOURCE ${_file} PROPERTY SKIP_AUTOMOC ON)

endforeach()

set(libmediaserver_SRCS
    ${upnpsdk_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/server/dlnaserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/dlnaserverdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/dmediaserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/dmediaservermngr.cpp
)

DISABLE_GCC_COMPILER_WARNINGS("13.2.99" "-Wtemplate-id-cdtor")

# For Unit-tests and plugin
add_library(mediaserverbackend STATIC ${libmediaserver_SRCS})
set_property(TARGET mediaserverbackend PROPERTY CXX_STANDARD 17)

if(MSVC)

    target_compile_options(mediaserverbackend PRIVATE /W0)

else()

    target_compile_options(mediaserverbackend PRIVATE -w)

endif()

target_link_libraries(mediaserverbackend
                      PRIVATE
                      digikamcore
                      ${CMAKE_THREAD_LIBS_INIT}
)

if(WIN32)

    target_link_libraries(mediaserverbackend
                          PRIVATE
                          # Defined in RulesWindows.cmake
                          ${WSOCK32_LIBRARY}
                          ${WS2_32_LIBRARY}
    )

endif()

# ---------------------------------------------------------------------------------------------------

set(mediaserverplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dmediaserverdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediaserverplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    MediaServer
                           SOURCES ${mediaserverplugin_SRCS}
                           DEPENDS mediaserverbackend
)
