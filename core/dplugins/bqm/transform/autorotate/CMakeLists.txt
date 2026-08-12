#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2025      by Michael Miller, <michael uderscore miller at msn dot com>
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

set(autorotateplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/autorotateplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/autorotate.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(NAME    AutoRotate
                       SOURCES ${autorotateplugin_SRCS}
                       DEPENDS opencv_core
                       opencv_objdetect
                       opencv_imgproc
                       opencv_imgcodecs
                       opencv_dnn
                       opencv_flann
)
