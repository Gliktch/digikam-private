#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libtimeadjust_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/clockphotodialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/detbyclockphotobutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjustcontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjustsettings.cpp
)

# Used by digikamcore
add_library(core_timeadjust_obj OBJECT ${libtimeadjust_SRCS})

target_compile_definitions(core_timeadjust_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
