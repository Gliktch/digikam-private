#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(fileactionmanager_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/metadatahub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadatahubmngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadatastatusbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileactionprogress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileactionmngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileactionmngr_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iteminfotasksplitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/databaseworkeriface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileworkeriface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fileactionimageinfolist.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Used by digikamgui
add_library(gui_fileactionmanager_obj OBJECT ${fileactionmanager_SRCS})

target_compile_definitions(gui_fileactionmanager_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
