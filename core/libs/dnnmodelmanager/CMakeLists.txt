#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2024-2025 by Michael Miller, <michael underscore miller at msn dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libdnnmodelmanager_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelinfocontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelnet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelyunet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnnmodelsface.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamcore
add_library(core_dnnmodelmanager_obj OBJECT ${libdnnmodelmanager_SRCS})

target_compile_definitions(core_dnnmodelmanager_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
