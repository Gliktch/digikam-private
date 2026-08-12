#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# =======================================================
# Facebook demo test tool

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)

set(fbdemo_SRCS ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookdemo/fbdemo.cpp
                ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/examples/facebookdemo/main.cpp
)

add_executable(fbdemo_gui ${fbdemo_SRCS})

target_link_libraries(fbdemo_gui
                      digikamcore

                      ${COMMON_TEST_LINK}
)

target_compile_definitions(fbdemo_gui PRIVATE signals=Q_SIGNALS slots=Q_SLOTS)

if(MSVC)

    target_compile_options(fbdemo_gui PRIVATE /W0)

else()

    target_compile_options(fbdemo_gui PRIVATE -w)

endif()
