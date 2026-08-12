#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

add_executable(dmediaserver_cli ${CMAKE_CURRENT_SOURCE_DIR}/dmediaserver_cli.cpp)

target_link_libraries(dmediaserver_cli

                      mediaserverbackend
                      digikamcore

                      ${COMMON_TEST_LINK}
 )

########################################################################
# CLI test tool from Platinum SDK

include_directories(${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/mediaserver/upnpsdk/Platinum/Source/Core
                    ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/mediaserver/upnpsdk/Platinum/Source/Platinum
                    ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/mediaserver/upnpsdk/Platinum/Source/Devices/MediaServer
                    ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/mediaserver/upnpsdk/Neptune/Source/Core
                    ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/mediaserver/server
)

add_executable(filemediaserver_cli
               ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/mediaserver/upnpsdk/Platinum/Source/Tests/FileMediaServer/FileMediaServerTest.cpp
)

set_property(TARGET filemediaserver_cli PROPERTY CXX_STANDARD 17)

target_link_libraries(filemediaserver_cli

                      digikamcore
                      mediaserverbackend

                      ${COMMON_TEST_LINK}
)
