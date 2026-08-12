#! /bin/bash

# Script to give a console with port CLI tool branched to Homebrew bundle repository
# Use this console to run code analysis tools in project/reports
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../config.sh
. ../common.sh

export QMAKE_FRAMEWORK_VERSION=6.7.2
export VERSION=$QMAKE_FRAMEWORK_VERSION
# export CMAKE_PREFIX_PATH=$INSTALL_PREFIX/Cellar/qt/6.7.2_1/
export BUILD_WITH_QT6=1
export Qt6_DIR=$INSTALL_PREFIX/Cellar/qt/6.7.2_1/
export CMAKE_BINARY=cmake
export DK_APPLE_PACKAGE_MANAGER=$DK_APPLE_PACKAGE_MANAGER
# QTPATHS="/opt/qt6/bin/qtpaths6"
# export CMAKE_BINARY=/opt/qt6/bin/cmake

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"

export PATH=$PATH:$INSTALL_PREFIX/Cellar/llvm@18/18.1.8/bin:$INSTALL_PREFIX/Cellar/clazy/1.12_1/bin:$INSTALL_PREFIX/opt/llvm/bin:$INSTALL_PREFIX/bin:$INSTALL_PREFIX/sbin

CommonChecks

#################################################################################################

# port
cd ../../../reports
/bin/zsh

#################################################################################################

export PATH=$ORIG_PATH
