# Script to build Fcitx-qt for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(EXTPREFIX_fcitx-qt "${EXTPREFIX}" )

ExternalProject_Add(ext_fcitx-core
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    GIT_REPOSITORY https://github.com/fcitx/fcitx5.git
    GIT_TAG 5.1.16

    CONFIGURE_COMMAND cd <SOURCE_DIR> &&
                      $(CMAKE_COMMAND) .
                         -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_fcitx-qt}
                         -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                         -DCMAKE_POLICY_VERSION_MINIMUM=3.5
                         -DENABLE_TESTING_ADDONS=OFF
                         -DENABLE_TEST=OFF
                         -DENABLE_ENCHANT=OFF
                         -DENABLE_DBUS=ON
                         -DENABLE_SERVER=OFF
                         -DENABLE_KEYBOARD=ON
                         -DUSE_SYSTEMD=ON
                         -DENABLE_XDGAUTOSTART=OFF
                         -DENABLE_EMOJI=OFF
                         -DENABLE_X11=ON
                         -DENABLE_WAYLAND=OFF
                         -DENABLE_LIBUUID=OFF
                         -DBUILD_SPELL_DICT=OFF
                         ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 0
    ALWAYS 0
)

if(${ENABLE_QTVERSION} MATCHES "5")

    message(STATUS "Compiling fcitx-qt for Qt5")

    ExternalProject_Add(ext_fcitx-qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        DEPENDS ext_fcitx-core


        GIT_REPOSITORY https://github.com/fcitx/fcitx5-qt.git
        GIT_TAG 5.1.11

        CONFIGURE_COMMAND cd <SOURCE_DIR> &&
                          $(CMAKE_COMMAND) .
                             -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_fcitx-qt}
                             -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                             -DCMAKE_POLICY_VERSION_MINIMUM=3.5
                             -DENABLE_LIBRARY=OFF
                             -DENABLE_QT4=OFF
                             -DENABLE_QT5=ON
                             -DENABLE_QT6=OFF
                             ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 0
        ALWAYS 0
    )

else()

    message(STATUS "Compiling fcitx-qt for Qt6")

    ExternalProject_Add(ext_fcitx-qt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        DEPENDS ext_fcitx-core

        GIT_REPOSITORY https://github.com/fcitx/fcitx5-qt.git
        GIT_TAG 5.1.11

        CONFIGURE_COMMAND cd <SOURCE_DIR> &&
                          $(CMAKE_COMMAND) .
                             -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                             -DCMAKE_POLICY_VERSION_MINIMUM=3.5
                             -DENABLE_LIBRARY=OFF
                             -DENABLE_QT4=OFF
                             -DENABLE_QT5=OFF
                             -DENABLE_QT6=ON
                             ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 0
        ALWAYS 0
    )

endif()
