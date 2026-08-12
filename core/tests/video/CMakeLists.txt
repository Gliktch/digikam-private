#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
)

if(HAVE_OPENGL)

    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::OpenGL,INTERFACE_INCLUDE_DIRECTORIES>)

    if(Qt6_FOUND)

        include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::OpenGLWidgets,INTERFACE_INCLUDE_DIRECTORIES>)

    endif()

endif()

# -------------------------------------------------

if(ENABLE_MEDIAPLAYER)

    add_executable(videothumb_cli ${CMAKE_CURRENT_SOURCE_DIR}/videothumb_cli.cpp)
    ecm_mark_nongui_executable(videothumb_cli)

    target_link_libraries(videothumb_cli
                          digikamcore

                          ${COMMON_TEST_LINK}
    )

    # -------------------------------------------------

    add_executable(videoplayer_gui ${CMAKE_CURRENT_SOURCE_DIR}/videoplayer_gui.cpp)

    target_link_libraries(videoplayer_gui

                          digikamcore

                          ${COMMON_TEST_LINK}
    )

    # -------------------------------------------------

    add_executable(videoslide_gui ${CMAKE_CURRENT_SOURCE_DIR}/videoslide_gui.cpp)

    target_link_libraries(videoslide_gui

                          digikamcore

                          ${COMMON_TEST_LINK}
    )


    # -------------------------------------------------


    add_executable(framesencoder_cli ${CMAKE_CURRENT_SOURCE_DIR}/framesencoder_cli.cpp)
    ecm_mark_nongui_executable(framesencoder_cli)

    target_link_libraries(framesencoder_cli

                          digikamcore

                          ${COMMON_TEST_LINK}
    )

endif()
