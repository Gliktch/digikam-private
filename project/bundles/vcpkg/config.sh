#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################

# About short paths to use with VCPKG, see https://github.com/microsoft/vcpkg/issues/24751
# /c/ is C:/ drive
# /e/ is E:/ drive

# Absolute path where are installed VCPKG scripts, portfiles, and configurations.
VCPKG_DIR="/e/vcpkg"

# Absolute path where are installed all compiled targets.
INSTALL_DIR="/e/dk"

# Absolute path where are downloaded all tarballs to compile.
DOWNLOAD_DIR="/e/d"

# Absolute path where are compiled all tarballs
BUILDING_DIR="/e/b"

VCPKG_TRIPLET="x64-windows"

VCPKG_COMMON_OPTIONS=(
"--disable-metrics"
"--no-print-usage"
"--triplet ${VCPKG_TRIPLET}"
"--x-buildtrees-root=${BUILDING_DIR}"
"--x-install-root=${INSTALL_DIR}"
"--downloads-root=${DOWNLOAD_DIR}"
"--vcpkg-root=${VCPKG_DIR}"
)

VCPKG_INSTALL_PREFIX=$INSTALL_DIR/$VCPKG_TRIPLET

# Do not enable binary cache of ports.
export VCPKG_BINARY_SOURCES=clear

#-------------------------------------------------------------------------------------------

# URL to git repository to checkout digiKam source code
# git protocol version which require a developer account with ssh keys.
DK_GITURL="git@invent.kde.org:graphics/digikam.git"
# https protocol version which give annonyous access.
#DK_GITURL="https://invent.kde.org/graphics/digikam.git"

# digiKam tarball information.
DK_URL="http://download.kde.org/stable/digikam"

# Location to build source code.
DK_BUILDTEMP=$BUILDING_DIR/dktemp

# Qt version to use in bundle and provided by VCPKG.
#DK_QTVERSION="5"
DK_QTVERSION="6"

# digiKam tag version from git. Official tarball do not include extra shared libraries.
# The list of tags can be listed with this url: https://invent.kde.org/graphics/digikam/-/tags
# If you want to package current implementation from git, use "master" as tag.
#DK_VERSION=v7.6.0
DK_VERSION=master
#DK_VERSION=qt5-maintenance

#DK_REVISION=7d64840905ba968de186a9f30bc3ccff64c89224

# Installer sub version to differentiates newer updates of the installer itself, even if the underlying application hasn’t changed.
DK_SUBVER=""

# Installer will include or not digiKam debug symbols
DK_DEBUG=0

# Sign bundles with GPG. Passphrase must be hosted in ~/.gnupg/dkorg-gpg-pwd.txt
DK_SIGN=0

# Upload automatically bundle to files.kde.org (pre-release only).
DK_UPLOAD=1
DK_UPLOADURL="digikam@tinami.kde.org"

# KDE frameworks version + Upload URL.
# See official release here: https://download.kde.org/stable/frameworks/

if [[ $DK_QTVERSION == 5 ]] ; then

    # KDE KF5 frameworks version.
    # See official release here: https://download.kde.org/stable/frameworks/
    DK_KDE_VERSION="5.116"

    # KDE Plasma version.
    # See official release here: https://download.kde.org/stable/plasma/
    DK_KP_VERSION="5.27.12"

    # KDE Application version.
    # See official release here: https://download.kde.org/stable/release-service/
    DK_KA_VERSION="24.05.1"

    DK_UPLOADDIR="/srv/archives/files/digikam/legacy"

else

    # KDE KF6 frameworks version.
    # See official release here: https://download.kde.org/stable/frameworks/
    DK_KDE_VERSION="v6.28.0"

    # KDE Plasma version.
    # See official release here: https://download.kde.org/stable/plasma/
    DK_KP_VERSION="v6.7.2"

    # KDE Application version.
    # See official release here: https://download.kde.org/stable/release-service/
    DK_KA_VERSION="v26.04.3"

    DK_UPLOADDIR="/srv/archives/files/digikam"

fi
