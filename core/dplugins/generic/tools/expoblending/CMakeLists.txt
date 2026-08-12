#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015,      Benjamin Girault, <benjamin dot girault at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/blendingdlg
                    ${CMAKE_CURRENT_SOURCE_DIR}/manager
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

set(expoblendingplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/expoblendingmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/expoblendingthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/alignbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/enfusebinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/expoblendingwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/expoblendingintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/expoblendingitemspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/expoblendingpreprocesspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/expoblendinglastpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/blendingdlg/enfusesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/blendingdlg/bracketstack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/blendingdlg/enfusestack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/blendingdlg/expoblendingdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expoblendingplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    ExpoBlending
                           SOURCES ${expoblendingplugin_SRCS}
)

add_subdirectory(icons)
