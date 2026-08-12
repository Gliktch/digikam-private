#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libnotificationmanager_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dnotificationwrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnotificationpopup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnotificationwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnotificationwidget_p.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF${QT_VERSION_MAJOR}Notifications_FOUND)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Notifications,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Used by digikamcore
add_library(core_notificationmanager_obj OBJECT ${libnotificationmanager_SRCS})

target_compile_definitions(core_notificationmanager_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
