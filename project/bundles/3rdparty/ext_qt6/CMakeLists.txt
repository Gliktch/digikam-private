# Script to build Qt6.
#
# Copyright (c) 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_qt ${EXTPREFIX})

set(QT_URL https://download.qt.io/official_releases/qt/6.11/6.11.1/single/qt-everywhere-src-6.11.1.tar.xz)
set(QT_HASH 25d4d1dd74c92b978f164e8f20805985)

IF(MSVC)

    # Windows with M$ Visual C++ compiler

    INCLUDE("config-qt6-windows.cmake")

    ExternalProject_Add(ext_qt6
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL ${QT_URL}
        URL_MD5 ${QT_HASH}

        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${QT_CONFIG}

        BUILD_COMMAND ${BUILD_CLI}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        ALWAYS 0
    )

ELSEIF(APPLE)

    INCLUDE("config-qt6-macos.cmake")

    ExternalProject_Add(ext_qt6
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL ${QT_URL}
        URL_MD5 ${QT_HASH}

        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${QT_CONFIG}

        BUILD_COMMAND ${BUILD_CLI}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        ALWAYS 0
    )

ELSE()

    # Linux with G++ compiler

    INCLUDE("config-qt6-linux.cmake")

    ExternalProject_Add(ext_qt6
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL ${QT_URL}
        URL_MD5 ${QT_HASH}

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt6-appimage-support.patch
                   && ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt6-openssl-linked.patch
                   && ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt611-fix-compile.patch

        CMAKE_ARGS -DOPENSSL_LIBS='-l${EXTPREFIX}/local/lib/libssl.a -l${EXTPREFIX}/local/lib/libcrypto.a -lpthread -ldl'
                   -DOPENSSL_INCDIR='${EXTPREFIX}/local/include/openssl/'
                   -DOPENSSL_LIBDIR='${EXTPREFIX}/local/lib'

        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${QT_CONFIG}

        # Compile with Ninja build framework.
        BUILD_COMMAND ninja

        INSTALL_COMMAND ninja install

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        ALWAYS 0
    )

ENDIF()
