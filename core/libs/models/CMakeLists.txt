#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Solid,INTERFACE_INCLUDE_DIRECTORIES>

    ${CMAKE_SOURCE_DIR}/core/app/main
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# ---------------------------------------------------------------------

set(libgenericmodels_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/categorizeditemmodel.cpp
)

# Used by digikamcore
add_library(core_digikamgenericmodels_obj OBJECT ${libgenericmodels_SRCS})

target_compile_definitions(core_digikamgenericmodels_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# ---------------------------------------------------------------------

set(libalbummodels_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/itemalbummodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemalbumfiltermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/abstractalbummodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/abstractalbummodel_counting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/abstractalbummodel_checkable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/abstractalbummodel_specific.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albummodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albummodel_date.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albummodel_search.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albummodel_tag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albumfiltermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albumfiltermodel_search.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albumfiltermodel_tagproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albumfiltermodel_checkable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albumfiltermodel_tagsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/albummodeldragdrophandler.cpp
)

# Used by digikamgui
add_library(gui_digikammodels_obj OBJECT ${libalbummodels_SRCS})

target_compile_definitions(gui_digikammodels_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
