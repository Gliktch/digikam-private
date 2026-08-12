# Script to build sane backends for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SET(EXTPREFIX_sane "${EXTPREFIX}")

ExternalProject_Add(ext_sane
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    GIT_REPOSITORY https://gitlab.com/sane-project/backends.git
    GIT_TAG 1.4.0

    INSTALL_DIR ${EXTPREFIX_sane}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix ${EXTPREFIX_sane} --with-gphoto2=no --with-v4l=no

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
