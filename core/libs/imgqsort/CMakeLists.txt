#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

kde_enable_exceptions()

set(libimgqsort_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/imagequalitysettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagequalitywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagequalityparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagequalitycalculator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagequalitythread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detectors/abstract_detector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detectors/aesthetic_detector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detectors/blur_detector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detectors/compression_detector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detectors/exposure_detector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detectors/noise_detector.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_imgqsort_obj OBJECT ${libimgqsort_SRCS})

target_compile_definitions(gui_imgqsort_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
