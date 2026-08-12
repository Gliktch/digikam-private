# Script to build Hugin for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

set(PREFIX_ext_hugin "${EXTPREFIX}")

set(Hugin_Conf -DDISABLE_DPKG=ON
               -DBUILD_HSI=OFF
               -DENABLE_LAPACK=OFF
               -DBUILD_HUGINSETUP=OFF
)

JoinListAsString("${Hugin_Conf}" " " BASH_OPTIONS)

ExternalProject_Add(ext_hugin
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    # To use official release tarball

    URL https://sourceforge.net/projects/hugin/files/hugin/hugin-2013.0/hugin-2013.0.0.tar.bz2/download
    URL_MD5 cc6c768df2aedc24a9a081754de05f39

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/hugin-appimage.patch

    INSTALL_DIR ${PREFIX_ext_hugin}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_hugin}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               ${Hugin_Conf}
               ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    ALWAYS 0
)
