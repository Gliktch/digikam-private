# Script to build FlowView plugin for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(FlowView_Conf ${GLOBAL_PROFILE})

if(${ENABLE_QTVERSION} MATCHES "6")

    set(FlowView_Conf ${FlowView_Conf} -DBUILD_WITH_QT6=ON)

else()

    set(FlowView_Conf ${FlowView_Conf} -DBUILD_WITH_QT6=OFF)

endif()

JoinListAsString("${FlowView_Conf}" " " BASH_OPTIONS)

set(PREFIX_ext_flowview "${EXTPREFIX}")

ExternalProject_Add(ext_flowview

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/cathaysia/digikamflowplugin.git

    PATCH_COMMAND git reset --hard &&
                  git submodule update --init &&
                  cd extern/QtFlowLayout &&
                  git checkout master

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_flowview}
               -DCMAKE_POLICY_VERSION_MINIMUM=3.5
               -DCMAKE_BUILD_TYPE=RelWithDebInfo
               ${FlowView_Conf}

    BUILD_IN_SOURCE 1

    UPDATE_COMMAND ""
)
