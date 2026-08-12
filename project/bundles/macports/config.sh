#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################

# Absolute path where are downloaded all tarballs to compile.
DOWNLOAD_DIR="`pwd`/temp.dwnld"

# Absolute path where are compiled all tarballs
BUILDING_DIR="`pwd`/temp.build"

########################################################################

# Build universal bundle (arm64 + Intel architecture). Work only with Silicon computer.
MP_UNIVERSAL=0

# Target macOS architecture: "x86_64" for Intel 64 bits, or "arm64" for Apple Silicon 64 bits.
# Used with the macport configuration.
HOST_ARCH="`uname -m`"

# Target bundle architecture. Used to configure CMake target and create prefix for the log files
# and the Macports install directory.
# Equal: "x86_64" for Intel 64 bits, "arm64" for Apple Silicon 64 bits, or "universal for both architectures.
TARGET_ARCH=$HOST_ARCH

if [[ $HOST_ARCH = "x86_64" ]] ; then

    # Minimum MacOS target for backward binary compatibility with Intel CPU
    # This require to install older MacOS SDKs with Xcode.
    # See this url to download a older SDK archive :
    #
    # https://github.com/alexey-lysiuk/macos-sdk
    #
    # Uncompress the archive to /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
    # and adjust the property "MinimumSDKVersion" from /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Info.plist
    # sudo /usr/libexec/PlistBuddy -c "Set MinimumSDKVersion 10.13" /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Info.plist
    #
    # Possible values:
    # 12.0  : Monterey     : tested   : Qt                                            5.13, 5.14, 5.15
    # 11.0  : BigSur       : tested   : Qt                                      5.12, 5.13, 5.14, 5.15
    # 10.15 : Catalina     : tested   : Qt                                5.11, 5.12, 5.13, 5.14, 5.15
    # 10.14 : Mojave       : tested   : Qt                     5.9, 5.10, 5.11, 5.12, 5.13, 5.14, 5.15
    # 10.13 : High Sierra  : tested   : Qt                     5.9, 5.10, 5.11, 5.12, 5.13, 5.14, 5.15
    # 10.12 : Sierra       : tested   : Qt                5.8, 5.9, 5.10, 5.11, 5.12, 5.13
    # 10.11 : El Capitan   : tested   : Qt 5.5, 5.6, 5.7, 5.8, 5.9, 5.10, 5.11
    # 10.10 : Yosemite     : tested   : Qt 5.5, 5.6, 5.7, 5.8, 5.9
    # 10.9  : Mavericks    : tested   : Qt 5.5, 5.6, 5.7, 5.8
    # 10.8  : MountainLion : tested   : Qt 5.5, 5.6, 5.7
    # 10.7  : Lion         : untested : Qt 5.5, 5.6
    # 10.6  : SnowLeopard  : untested : ???
    #
    # Older values cannot be set as it do not support x86_64.
    OSX_MIN_TARGET="10.15"

elif [[ $HOST_ARCH = "arm64" ]] ; then

    # Apple Silicon is supported since macOS BigSur
    OSX_MIN_TARGET="11.3"

    if [[ $MP_UNIVERSAL = 1 ]] ; then

        TARGET_ARCH=universal

    fi

else

    echo "Unsupported or invalid target architecture ($HOST_ARCH)..."
    exit -1

fi

echo "Host Architecture  : $HOST_ARCH"
echo "Target Architecture: $TARGET_ARCH"

# Directory to build and install Macports packages.
INSTALL_PREFIX="/opt/digikam.org.$TARGET_ARCH"
# Local install prefix which do not require sudo right
#INSTALL_PREFIX="`pwd`/digikam.org.$TARGET_DIR_SUF"

# Directory where target bundle contents will be installed.
RELOCATE_PREFIX="/Applications/digiKam.org"

# Macports configuration
MP_URL="https://distfiles.macports.org/MacPorts/"
MP_BUILDTEMP=~/mptemp

# Uncomment this line to force a specific version of Macports to use, else latest will be used.
#MP_VERSION="2.3.3"

########################################################################

# URL to git repository to checkout digiKam source code
# git protocol version which require a developer account with ssh keys.
DK_GITURL="git@invent.kde.org:graphics/digikam.git"
# https protocol version which give annonyous access.
#DK_GITURL="https://invent.kde.org/graphics/digikam.git"

# digiKam tarball information
DK_URL="http://download.kde.org/stable/digikam"

# Location to build source code.
DK_BUILDTEMP=~/dktemp

# Qt version to use in bundle and provided by Macports.
if [[ $HOST_ARCH = "x86_64" ]] ; then

    DK_QTVERSION="5"

else

    DK_QTVERSION="6"

fi

# Mariadb version to install for Qt SQL plugin.
DK_MARIADB_VERSION="10.11"

MP_MARIADB_VARIANT="+mariadb$DK_MARIADB_VERSION"
MP_MARIADB_VARIANT=${MP_MARIADB_VARIANT//./_}
MARIADB_SUFFIX="-$DK_MARIADB_VERSION"

# digiKam tag version from git. Official tarball do not include extra shared libraries.
# The list of tags can be listed with this url: https://invent.kde.org/graphics/digikam/-/tags
# If you want to package current implementation from git, use "master" as tag.
#DK_VERSION=v8.4.0
DK_VERSION=master
#DK_VERSION=development/dplugins

# Installer sub version to differentiates newer updates of the installer itself, even if the underlying application hasn’t changed.
#DK_SUBVER="-01"

# needed to separate Macports from Homebrew
DK_APPLE_PACKAGE_MANAGER="macports"

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
    DK_KP_VERSION="5.27.11"

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

    DK_UPLOADDIR="/srv/archives/files/digikam/"

fi
