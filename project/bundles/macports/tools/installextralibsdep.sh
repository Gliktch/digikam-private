#! /bin/bash

# Script to install extra libs dependency pass as name as first argument (ex: "kimageformats")
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

if [ -z "$1" ] ; then

    echo "Pass the name of extra libs dependency to install as first argument (aka 'kimageformats' for ex.)"
    exit
fi

#################################################################################################
# Pre-processing checks

cd ..

. ./config.sh
. ./common.sh
StartScript
ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores
OsxCodeName
#RegisterRemoteServers

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/libexec/qt5/bin:$ORIG_PATH

#################################################################################################

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILDING_DIR ] ; then
    mkdir $BUILDING_DIR
fi
if [ ! -d $DOWNLOAD_DIR ] ; then
    mkdir $DOWNLOAD_DIR
fi

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
       -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
       -DINSTALL_ROOT=$INSTALL_PREFIX \
       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
       -DKA_VERSION=$DK_KA_VERSION \
       -DKDE_VERSION=$DK_KDE_VERSION \
       -DENABLE_QTVERSION=$DK_QTVERSION \
       -Wno-dev

cmake --build . --config RelWithDebInfo --target ext_$1 -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
