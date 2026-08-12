#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(akonadiiface_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/akonadiiface.cpp)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KPim${QT_VERSION_MAJOR}AkonadiContact_FOUND)
    include_directories(
        $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::AkonadiCore,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Contacts,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    )
elseif(KPim${QT_VERSION_MAJOR}AkonadiContactCore_FOUND)
    include_directories(
        $<TARGET_PROPERTY:KPim${QT_VERSION_MAJOR}::AkonadiCore,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Contacts,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

add_library(core_akonadiiface_obj OBJECT ${akonadiiface_SRCS})

target_compile_definitions(core_akonadiiface_obj
                           PRIVATE
                           digikamcore_EXPORTS
)


add_library(akonadiiface STATIC $<TARGET_OBJECTS:core_akonadiiface_obj>)

target_link_libraries(akonadiiface PRIVATE Qt${QT_VERSION_MAJOR}::Core KF${QT_VERSION_MAJOR}::I18n)

if(KPim${QT_VERSION_MAJOR}AkonadiContact_FOUND)
    target_include_directories(core_akonadiiface_obj PRIVATE $<TARGET_PROPERTY:KPim${QT_VERSION_MAJOR}::AkonadiContact,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(akonadiiface PRIVATE KPim${QT_VERSION_MAJOR}::AkonadiContact KF${QT_VERSION_MAJOR}::Contacts)
elseif(KPim${QT_VERSION_MAJOR}AkonadiContactCore_FOUND)
    target_include_directories(core_akonadiiface_obj PRIVATE $<TARGET_PROPERTY:KPim${QT_VERSION_MAJOR}::AkonadiContactCore,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(akonadiiface PRIVATE KPim${QT_VERSION_MAJOR}::AkonadiContactCore KF${QT_VERSION_MAJOR}::Contacts)
endif()
