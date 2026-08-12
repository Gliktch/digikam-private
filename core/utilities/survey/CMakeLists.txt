#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libsurvey_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/surveystack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/surveywindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/surveywindow_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/surveywindow_setup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/surveywindow_tools.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

# Used by digikamgui
add_library(gui_survey_obj OBJECT ${libsurvey_SRCS})

target_compile_definitions(gui_survey_obj
                           PRIVATE
                           digikamgui_EXPORTS
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/surveywindowui5.rc DESTINATION ${KDE_INSTALL_FULL_KXMLGUIDIR}/digikam)
