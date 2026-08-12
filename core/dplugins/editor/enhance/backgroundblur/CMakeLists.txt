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

set(backgroundblurtoolplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/backgroundblurtoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backgroundblurfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backgroundblurtool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(NAME    BackgroundBlurTool
                          SOURCES ${backgroundblurtoolplugin_SRCS}
                          DEPENDS opencv_core
                                  opencv_objdetect
                                  opencv_imgproc
                                  opencv_imgcodecs
                                  opencv_dnn
                                  opencv_flann
)
