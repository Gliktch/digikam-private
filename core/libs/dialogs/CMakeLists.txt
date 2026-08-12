#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

if(LensFun_FOUND)
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Solid,INTERFACE_INCLUDE_DIRECTORIES>

    ${CMAKE_BINARY_DIR}/core/app/utils
)

if(KF${QT_VERSION_MAJOR}Sonnet_FOUND)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::SonnetCore,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)

set(libdialogs_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dprogressdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dbusydlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filesaveoptionsdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iccprofileinfodlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagedialog_preview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagedialog_iconprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imagedialog_tooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/infodlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg_features.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg_libraries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg_buildtools.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg_manifest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsinfodlg_opencv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawcameradlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solidhardwaredlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dconfigdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dconfigdlgmngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dconfigdlgmodels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dconfigdlgview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dconfigdlgview_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dconfigdlgwidgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmessagebox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dsplashscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webbrowserdlg.cpp
)

# Used by digikamcore
add_library(core_digikamdialogs_obj OBJECT ${libdialogs_SRCS})

target_compile_definitions(core_digikamdialogs_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

set(libdeletedialog_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/deletedialog.cpp
)

# Used by digikamgui
add_library(gui_digikamdeletedialog_obj OBJECT ${libdeletedialog_SRCS})

target_compile_definitions(gui_digikamdeletedialog_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
