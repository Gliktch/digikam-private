#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::WindowSystem,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF${QT_VERSION_MAJOR}KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

# ------------------------------------------------------------------------------------

set(libeditorwidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/imageguidewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/imagepreviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/previewtoolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/previewlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/imageregionwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/imageregionitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/rubberitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/canvas.cpp
)

set(libeditordlg_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/colorcorrectiondlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/softproofdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/versioningpromptusersavedlg.cpp
)

set(libeditorcore_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/undocache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/undoaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/undomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editorcore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iccpostloadingmanager.cpp
)

set(libeditoriface_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/editortool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/editortooliface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/editorstackview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/editortoolsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/editorwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/imageiface.cpp
)

# Used by digikamcore
add_library(core_imageeditor_obj OBJECT ${libeditorcore_SRCS}
                                        ${libeditordlg_SRCS}
                                        ${libeditoriface_SRCS}
                                        ${libeditorwidgets_SRCS}
)

target_compile_definitions(core_imageeditor_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# ------------------------------------------------------------------------------------

set(libeditorgui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main/imagewindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/imagewindow_setup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/imagewindow_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/imagewindow_import.cpp
)

# Used by digikamgui
add_library(gui_imageeditorgui_obj OBJECT ${libeditorgui_SRCS})

target_compile_definitions(gui_imageeditorgui_obj
                           PRIVATE
                           digikamgui_EXPORTS
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/main/imageeditorui5.rc DESTINATION ${KDE_INSTALL_FULL_KXMLGUIDIR}/digikam)
