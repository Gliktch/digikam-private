#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/manager
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

set(printcreatorplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/gimpbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/templateicon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/atkinspagelayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/atkinspagelayoutnode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/atkinspagelayouttree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/advprintphoto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/advprintsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/advprintthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/advprinttask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintalbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintphotopage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcaptionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcroppage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintoutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcustomdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcropframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printcreatorplugin.cpp
)

ki18n_wrap_ui(printcreatorplugin_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintphotopage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintcroppage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintcaptionpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintcustomlayout.ui
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    PrintCreator
                           SOURCES ${printcreatorplugin_SRCS}
                           DEPENDS Qt${QT_VERSION_MAJOR}::PrintSupport
)

install(DIRECTORY templates DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/)
