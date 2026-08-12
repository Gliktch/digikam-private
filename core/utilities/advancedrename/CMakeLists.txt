#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libadvancedrename_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/advancedrenamedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/advancedrenameinput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/advancedrenamemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/advancedrenameprocessdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/advancedrenamewidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/common/dynamiclayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/highlighter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/modifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/option.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/parseresults.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/rule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/ruledialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/tooltipcreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/tooltipdialog.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/parser/importrenameparser.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/casemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/dateformatmodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/defaultvaluemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/rangemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/removedoublesmodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/replacemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/trimmedmodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/uniquemodifier.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/cameranameoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/dateoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/directorynameoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/filepropertiesoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/metadataoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/sequencenumberoption.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/databaseoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/dbheaderlistitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/dbkeyscollection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/dbkeyselector.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/commonkeys.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/metadatakeys.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/positionkeys.cpp
)

set(libadvancedrename_UI
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/fillmodifierdialogwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/rangemodifierdialogwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/replacemodifierdialogwidget.ui

    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/dateoptiondialogwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/sequencenumberoptiondialogwidget.ui
)

ki18n_wrap_ui(libadvancedrename_SRCS ${libadvancedrename_UI})

# TODO for internal use, prefer complete include paths instead of this
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers
)

include_directories(
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_advancedrename_obj OBJECT ${libadvancedrename_SRCS})

target_compile_definitions(gui_advancedrename_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
