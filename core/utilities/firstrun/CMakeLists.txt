#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libfirstrun_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/firstrundlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/welcomepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/databasepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tooltipspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/previewpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openfilepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadatapage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/startscanpage.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

if(NOT Qt6_FOUND)

    set(libfirstrun_SRCS
        ${libfirstrun_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/migratefromdigikam4page.cpp
    )

endif()

# Used by digikamgui
add_library(gui_firstrun_obj OBJECT ${libfirstrun_SRCS})

target_compile_definitions(gui_firstrun_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
