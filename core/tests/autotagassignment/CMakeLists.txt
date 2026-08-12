#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

# -----------------------------------------------------------------------------

set(detect_object_cli_SRCS  ${CMAKE_CURRENT_SOURCE_DIR}/detect_object_cli.cpp)

add_executable(detect_object_cli ${detect_object_cli_SRCS})

target_link_libraries(detect_object_cli

                      digikamcore
                      digikamdatabase
                      digikamgui

                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::ConfigCore
                      KF${QT_VERSION_MAJOR}::Service
                      ${COMMON_TEST_LINK}
)

# -----------------------------------------------------------------------------

set(detect_object_gui_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/detect_object_gui.cpp)

add_executable(detect_object_gui ${detect_object_gui_SRCS})

target_link_libraries(detect_object_gui

                      digikamcore
                      digikamdatabase
                      digikamgui

                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::ConfigCore
                      KF${QT_VERSION_MAJOR}::Service
                      ${COMMON_TEST_LINK}
)

# -----------------------------------------------------------------------------

set(detect_object_batch_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/detect_object_batch_cli.cpp)
add_executable(detect_object_batch_cli ${detect_object_batch_cli_SRCS})
target_link_libraries(detect_object_batch_cli

                      digikamcore
                      digikamdatabase
                      digikamgui

                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::ConfigCore
                      KF${QT_VERSION_MAJOR}::Service
                      ${COMMON_TEST_LINK}
)



# -----------------------------------------------------------------------------

set(autotagging_batch_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/autotagging_batch_cli.cpp)
add_executable(autotagging_batch_cli ${autotagging_batch_cli_SRCS})
target_link_libraries(autotagging_batch_cli

                      digikamcore
                      digikamdatabase
                      digikamgui

                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::ConfigCore
                      KF${QT_VERSION_MAJOR}::Service
                      ${COMMON_TEST_LINK}
)

# -----------------------------------------------------------------------------

set(resnet_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/resnet_cli.cpp)
add_executable(resnet_cli ${resnet_cli_SRCS})
target_link_libraries(resnet_cli

                      digikamcore
                      digikamdatabase
                      digikamgui

                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::ConfigCore
                      KF${QT_VERSION_MAJOR}::Service
                      ${COMMON_TEST_LINK}
)
