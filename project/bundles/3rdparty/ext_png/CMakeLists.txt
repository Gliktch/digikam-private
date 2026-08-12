# Script to build libpng for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SET(PREFIX_ext_png "${EXTPREFIX}")

ExternalProject_Add(ext_png

    GIT_REPOSITORY https://github.com/pnggroup/libpng.git
    GIT_TAG 1.6.58

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_png}

    UPDATE_COMMAND ""
    ALWAYS 0
)
