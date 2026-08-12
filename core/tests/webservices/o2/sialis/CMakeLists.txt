#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Qml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)


# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)

# =======================================================
# Twitter demo test tool

set(sialis_SRCS ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/sialis/tweetmodel.cpp
                ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/sialis/twitterapi.cpp
                ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/sialis/main.cpp
)

qt5_add_resources(QT_RESOURCES ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/sialis/main.qml
)

add_executable(sialis_gui ${sialis_SRCS})

target_link_libraries(sialis_gui

                      digikamcore

                      ${COMMON_TEST_LINK}

                      Qt5::Qml
                      Qt5::WebView
)

target_compile_definitions(sialis_gui PRIVATE signals=Q_SIGNALS slots=Q_SLOTS)

if(MSVC)

    target_compile_options(sialis_gui PRIVATE /W0)

else()

    target_compile_options(sialis_gui PRIVATE -w)

endif()
