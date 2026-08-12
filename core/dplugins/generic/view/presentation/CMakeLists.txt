#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

if(HAVE_MEDIAPLAYER)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/audio)

    if(Qt6_FOUND AND ENABLE_QTMULTIMEDIA)

        include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::MultimediaWidgets,INTERFACE_INCLUDE_DIRECTORIES>)

        set(QTMEDIA_BACKEND Qt${QT_VERSION_MAJOR}::Multimedia
                            Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        )

    else()

        set(QTMEDIA_BACKEND ${MEDIAPLAYER_LIBRARIES})

    endif()

endif()

if(HAVE_OPENGL)

    if(APPLE)

        # Under MacOS 10.14 (Mojave), OpenGL API start to be deprecated in favor to Apple Metal API.
        # https://appleinsider.com/articles/18/06/28/why-macos-mojave-requires-metal----and-deprecates-opengl
        add_definitions(-DGL_SILENCE_DEPRECATION)

    endif()

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opengl)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::OpenGL,INTERFACE_INCLUDE_DIRECTORIES>)

    set(QT_OPENGL_LIBRARIES Qt${QT_VERSION_MAJOR}::OpenGL)

    if (Qt6_FOUND)
        include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::OpenGLWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
        set(QT_OPENGL_LIBRARIES ${QT_OPENGL_LIBRARIES} Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
    endif()

endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(presentationplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_captionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_advpage.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/presentationctrlwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/presentationwidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/common/presentationloader.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentationdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_mainpage.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/presentationmngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/presentationplugin.cpp
)

if(HAVE_OPENGL)

    set(presentationplugin_SRCS
        ${presentationplugin_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/presentationgl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/presentationkb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/kbeffect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/kbimageloader.cpp
    )

endif()

if(HAVE_MEDIAPLAYER)

    set(presentationplugin_SRCS
        ${presentationplugin_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentation_audiopage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentationaudiolist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentationaudiowidget.cpp
    )

    ki18n_wrap_ui(presentationplugin_SRCS
                  ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentation_audiopage.ui
                  ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentationaudiowidget.ui
    )

endif()

ki18n_wrap_ui(presentationplugin_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_mainpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_captionpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_advpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/widgets/presentationctrlwidget.ui
)


if(HAVE_OPENGL)

    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Presentation
                               SOURCES ${presentationplugin_SRCS}
                               DEPENDS ${QT_OPENGL_LIBRARIES}
                                       ${QTMEDIA_BACKEND}
                                       ${OPENGL_LIBRARIES}
    )

else()

    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Presentation
                               SOURCES ${presentationplugin_SRCS}
                               DEPENDS ${QTMEDIA_BACKEND}

    )

endif()
