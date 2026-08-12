# Script to build libgphoto for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SET(EXTPREFIX_tiff "${EXTPREFIX}" )

ExternalProject_Add(ext_tiff
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    GIT_REPOSITORY https://gitlab.com/libtiff/libtiff.git

    GIT_TAG v4.7.2

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix ${EXTPREFIX_tiff}
    UPDATE_COMMAND ""
    ALWAYS 0
)

