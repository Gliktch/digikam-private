#!/bin/bash

# Configuration for script to build AppImage bundle.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################

# NOTE: include common.sh header first.
CheckSystemReleaseID

# digiKam tag version from git. Official tarball do not include extra shared libraries.
# The list of tags can be listed with this url: https://invent.kde.org/graphics/digikam/-/tags
# If you want to package current implementation from git, use "master" as tag.
#DK_VERSION=v7.0.0
DK_VERSION=master
#DK_VERSION=work/michmill/8_7_0-bqm-autorotate

# Absolute path where are installed all software components
INSTALL_DIR="/usr"

# Absolute path where are bundled all software components
APP_IMG_DIR="/digikam.appdir"

########################################################################

# URL to git repository to checkout digiKam source code
# git protocol version which require a developer account with ssh keys.
DK_GITURL="git@invent.kde.org:graphics/digikam.git"
# https protocol version which give annonyous access.
#DK_GITURL="https://invent.kde.org/graphics/digikam.git"

# Location to build source code.
DK_BUILDTEMP=$BUILDING_DIR/dktemp

# Qt version to use in bundle. Possible values:
# - 5:    stable Qt5 release.
# - 6:    stable Qt6 release.

#DK_QTVERSION="5"
DK_QTVERSION="6"

# Installer will include or not digiKam debug symbols
DK_DEBUG=0

# Installer sub version to differentiates newer updates of the installer itself, even if the underlying application hasn’t changed.
#DK_SUBVER="-01"

# Sign bundles with GPG. Passphrase must be hosted in ~/.gnupg/dkorg-gpg-pwd.txt
DK_SIGN=0

# Upload automatically bundle to files.kde.org (pre-release only).
DK_UPLOAD=1
DK_UPLOADURL="digikam@tinami.kde.org"

# KDE frameworks version + Upload URL.
# See official release here: https://download.kde.org/stable/frameworks/

if [[ $DK_QTVERSION == 5 ]] ; then

    # Absolute path where are downloaded all tarballs to compile.
    DOWNLOAD_DIR="/d"

    # Absolute path where are compiled all tarballs
    BUILDING_DIR="/b/$DK_VERSION"

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

    # Absolute path where are downloaded all tarballs to compile.
    DOWNLOAD_DIR="/mnt/data/d"

    # Absolute path where are compiled all tarballs
    BUILDING_DIR="/mnt/data/b/$DK_VERSION"

    DK_UPLOADDIR="/srv/archives/files/digikam/"

fi

