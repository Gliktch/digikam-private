#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>
)


# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)

# =======================================================
# Facebook demo test tool

set(fbinter_SRCS ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookexternalinterceptordemo/fbdemo.cpp
                 ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookexternalinterceptordemo/main.cpp
                 ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookexternalinterceptordemo/webenginepage.cpp
                 ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookexternalinterceptordemo/webwindow.cpp
)

qt5_wrap_ui(fbinter_SRCS
            ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookexternalinterceptordemo/webwindow.ui
)

add_executable(fbinter_gui ${fbinter_SRCS})

target_link_libraries(fbinter_gui

                      digikamcore

                      ${COMMON_TEST_LINK}
                      Qt5::WebEngineWidgets
)

target_compile_definitions(fbinter_gui PRIVATE signals=Q_SIGNALS slots=Q_SLOTS)

if(MSVC)

    target_compile_options(fbinter_gui PRIVATE /W0)

else()

    target_compile_options(fbinter_gui PRIVATE -w)

endif()
