#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libfacemanagement_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/database/faceutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/faceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/facegroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/facegroup_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/assignnamewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/assignnamewidget_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/assignnamewidgetstates.cpp

    # new face engine
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/facepipelinebase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/facepipelinepackagebase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/recognize/facepipelinerecognize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/edit/facepipelineedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/retrain/facepipelineretrain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/detectrecognize/facepipelinedetectrecognize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/reset/facepipelinereset.cpp

    # dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/facetrainingupgradedlg.cpp

    # background processes
    ${CMAKE_CURRENT_SOURCE_DIR}/backgroundprocesses/facebackgroundrecognition.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Used by digikamgui
add_library(gui_facemanagement_obj OBJECT ${libfacemanagement_SRCS})

target_compile_definitions(gui_facemanagement_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
