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
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

add_executable(ocrTesseract_cli  ${CMAKE_CURRENT_SOURCE_DIR}/ocrTesseract_cli.cpp)
target_link_libraries(ocrTesseract_cli

                      textconverterbackend
                      digikamcore

                      ${COMMON_TEST_LINK}
)

add_executable(ocrTesseract_process   ${CMAKE_CURRENT_SOURCE_DIR}/ocrTesseract_process_cli.cpp)
target_link_libraries(ocrTesseract_process

                      textconverterbackend
                      digikamcore

                      ${COMMON_TEST_LINK}
)

add_executable(tesseract_version ${CMAKE_CURRENT_SOURCE_DIR}/tesseractversion_cli.cpp)
target_link_libraries(tesseract_version

                      textconverterbackend
                      digikamcore

                      ${COMMON_TEST_LINK}
)
