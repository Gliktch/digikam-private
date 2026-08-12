#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libmaintenance_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/iteminfojob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/iteminfoalbumsjob.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/manager/maintenancedata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/maintenancethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/maintenancetool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/maintenancemngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/maintenancesettings.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/main/maintenancedlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/maintenancedlg_settings.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/metaremover/metadataremovetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/metaremover/metadataremover.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/metasync/metadatasynctask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/metasync/metadatasynchronizer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/duplicates/duplicatesfinder.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/dbcleaner/databasetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/dbcleaner/dbcleaner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/dbcleaner/dbshrinkdialog.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/dbscan/newitemsfinder.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/thumbs/thumbsgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/thumbs/thumbstask.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/facesmanagement/facesengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/facesmanagement/facesengine_p.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/fingerprints/fingerprintsgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/fingerprints/fingerprintstask.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/imgqsort/imagequalitysorter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/imgqsort/imagequalitytask.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_maintenance_obj OBJECT ${libmaintenance_SRCS})

target_compile_definitions(gui_maintenance_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
