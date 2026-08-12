#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-extra-semi")

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050E00
    -DQT_NO_KEYWORDS
    -DQT_NO_FOREACH
)

include_directories(${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/src)

if(NOT APPLE)

    add_subdirectory(fbdemo)

# NOTE: Do not compile anymore on CI
#   add_subdirectory(fbinter)

endif()

if(Qt5Qml_FOUND AND Qt5WebView_FOUND)
    add_subdirectory(sialis)
endif()

add_subdirectory(twdemo)
