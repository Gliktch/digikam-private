#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050E00
)

set(twitterplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/twittermpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/twitternewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/twitterplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/twittertalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/twitterwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/twitterwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Twitter
                           SOURCES ${twitterplugin_SRCS}
)
