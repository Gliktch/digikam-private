#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

if(APPLE)
    # Under MacOS 10.14 (Mojave), OpenGL API start to be deprecated in favor to Apple Metal API.
    # https://appleinsider.com/articles/18/06/28/why-macos-mojave-requires-metal----and-deprecates-opengl
    add_definitions(-DGL_SILENCE_DEPRECATION)

endif()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::OpenGL,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(QT_OPENGL_LIBRARIES Qt${QT_VERSION_MAJOR}::OpenGL)

if (Qt6_FOUND)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::OpenGLWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
    set(QT_OPENGL_LIBRARIES ${QT_OPENGL_LIBRARIES} Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif ()

set(glviewerplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewerplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewertexture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewertimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewerwidget.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    GLViewer
                           SOURCES ${glviewerplugin_SRCS}
                           DEPENDS ${QT_OPENGL_LIBRARIES}
                                   ${OPENGL_LIBRARIES}
)
