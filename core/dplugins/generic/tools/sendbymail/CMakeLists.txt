#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools

                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/manager
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

set(sendbymailplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/imageresizethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/imageresizejob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/mailsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/mailprocess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailalbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailsettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/outlookbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/thunderbirdbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/evolutionbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/kmailbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/sylpheedbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/netscapebinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/clawsmailbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/balsabinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sendbymailplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    SendByMail
                           SOURCES ${sendbymailplugin_SRCS}
)
