#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories(
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(textconverterbackend_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/ocroptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tesseractbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textconverterdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textconverterlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textconvertersettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ocrtesseractengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textconvertertask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textconverterthread.cpp
)

# For Unit-tests and plugin
add_library(textconverterbackend STATIC ${textconverterbackend_SRCS})

target_link_libraries(textconverterbackend
                      PRIVATE
                      digikamcore
)

set(textconverterplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/textconverterplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    TextConverter
                           SOURCES ${textconverterplugin_SRCS}
                           DEPENDS textconverterbackend
)
