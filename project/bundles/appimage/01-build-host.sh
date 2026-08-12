#!/bin/bash

# Script to build a Linux Host installation to compile an AppImage bundle of digiKam.
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-host.full.log) 2>&1

#################################################################################################

echo "01-build-host.sh : build a Linux host installation to compile an AppImage bundle."
echo "---------------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./common.sh
. ./config.sh
ChecksRunAsRoot
StartScript
ChecksCPUCores
ChecksPhyMemory
HostAdjustments
RegisterRemoteServers
ORIG_WD="`pwd`"

#################################################################################################

. ./host_ubuntu.inc

# Clean up previous openssl and libicu install

rm -fr /usr/local/lib/libssl.a       || true
rm -fr /usr/local/lib/libcrypto.a    || true
rm -fr /usr/local/include/openssl    || true

rm -fr /usr/local/lib/libicu*.a      || true
rm -fr /usr/local/lib/icu            || true
rm -fr /usr/local/lib/pkgconfig/icu* || true
rm -fr /usr/local/include/unicode    || true

#################################################################################################

echo -e "---------- Prepare Linux host to Compile Extra Dependencies\n"

# Make sure we build from the /, parts of this script depends on that. We also need to run as root...
cd /

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILDING_DIR ] ; then
    mkdir -p $BUILDING_DIR
fi

if [ ! -d $DOWNLOAD_DIR ] ; then
    mkdir -p $DOWNLOAD_DIR
fi

#################################################################################################

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=/usr \
      -DINSTALL_ROOT=/usr \
      -DENABLE_QTVERSION=$DK_QTVERSION \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION

# Install new cmake recent version and shared lib

cmake --build . --config RelWithDebInfo --target ext_cmake           -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_jasper          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_libde265        -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_libjxl          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_libaom          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_libavif         -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_ffmpeg          -- -j$CPU_CORES

#################################################################################################

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=/usr \
      -DINSTALL_ROOT=/usr \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION

# Low level libraries and Qt dependencies
# NOTE: The order to compile each component here is very important.

# TODO: more recent libicu do not link yet with Qt6
#cmake --build . --config RelWithDebInfo --target ext_libicu        -- -j$CPU_CORES

cmake --build . --config RelWithDebInfo --target ext_openssl         -- -j$CPU_CORES

# NOTE: QtWebEngine require 4Gb of RAM by CPU cores to compile in parallel.

QT_CORES=$((PHY_MEM / 4 / 2))
echo "Qt will be compiled with $QT_CORES CPU cores."

cmake --build . --config RelWithDebInfo --target ext_qt$DK_QTVERSION -- -j$QT_CORES    # depend of tiff, png, jpeg

# Clean up previous openssl install

rm -fr /usr/local/lib/libssl.a    || true
rm -fr /usr/local/lib/libcrypto.a || true
rm -fr /usr/local/include/openssl || true

cmake --build . --config RelWithDebInfo --target ext_imagemagick     -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_opencv          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_heif            -- -j$CPU_CORES

#################################################################################################

TerminateScript
