#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(Qt${QT_VERSION_MAJOR} REQUIRED
             NO_MODULE COMPONENTS
             Core
             Concurrent
             Widgets
             Gui
             Sql
             Xml
             PrintSupport
             Network
             NetworkAuth
             Svg
)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED
                                   NO_MODULE
                                   COMPONENTS
                                   WebEngineWidgets
)

find_package(Qt${QT_VERSION_MAJOR}
             OPTIONAL_COMPONENTS
             DBus
             OpenGL
)

if(Qt6_FOUND)

    find_package(Qt${QT_VERSION_MAJOR} REQUIRED
                 NO_MODULE COMPONENTS
                 StateMachine
                 SvgWidgets
    )

    find_package(Qt${QT_VERSION_MAJOR}
                 OPTIONAL_COMPONENTS
                 OpenGLWidgets
    )

else()

    find_package(Qt${QT_VERSION_MAJOR}
                 OPTIONAL_COMPONENTS
                 XmlPatterns                # For Rajce plugin
    )

endif()

if(ENABLE_DBUS)

    if(NOT Qt${QT_VERSION_MAJOR}DBus_FOUND)

        set(ENABLE_DBUS OFF)

    endif()

endif()

# Qt Dependencies For unit tests and CLI test tools

if(BUILD_TESTING)

    find_package(Qt${QT_VERSION_MAJOR} REQUIRED
                                       NO_MODULE
                                       COMPONENTS
                                       Test
    )

    find_package(Qt${QT_VERSION_MAJOR}
                 QUIET
                 OPTIONAL_COMPONENTS
                 Qml WebView  # Optional, for 'sialis' O2 library test tool.
    )

endif()
