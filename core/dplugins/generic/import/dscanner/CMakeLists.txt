#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(Qt6_FOUND)

    include_directories($<TARGET_PROPERTY:KSaneWidgets6,INTERFACE_INCLUDE_DIRECTORIES>)

else()

    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Sane,INTERFACE_INCLUDE_DIRECTORIES>)

endif()

set(dscannerplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dscannerplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scandialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/saveimgthread.cpp
)

if(Qt6_FOUND)

    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    DigitalScanner
                               SOURCES ${dscannerplugin_SRCS}
                               DEPENDS KSaneWidgets6
    )

else()

    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    DigitalScanner
                               SOURCES ${dscannerplugin_SRCS}
                               DEPENDS KF${QT_VERSION_MAJOR}::Sane
    )

endif()
