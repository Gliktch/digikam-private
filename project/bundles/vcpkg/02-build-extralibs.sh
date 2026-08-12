#!/bin/bash

# Script to build extra libraries using VCPKG.
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
exec > >(tee ./logs/build-extralibs.full.log) 2>&1

#################################################################################################

echo "02-build-extralibs.sh : build extra libraries."
echo "----------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./common.sh
. ./config.sh
StartScript
ChecksCPUCores
ChecksCmakeBin
RegisterRemoteServers
AppendVCPKGPaths

ORIG_WD="`pwd`"

#################################################################################################

if [ ! -d $BUILDING_DIR/dk_cmake ] ; then
    mkdir -p $BUILDING_DIR/dk_cmake
fi

cd $BUILDING_DIR/dk_cmake

rm -rf $BUILDING_DIR/dk_cmake/* || true

"${CMAKE_BIN}" $ORIG_WD/../3rdparty \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=$VCPKG_TRIPLET \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_COLOR_MAKEFILE=ON \
      -DCMAKE_INSTALL_PREFIX=$VCPKG_INSTALL_PREFIX \
      -DINSTALL_ROOT=$VCPKG_INSTALL_PREFIX \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION \
      -Wno-dev

# NOTE: The order to compile each component here is very important.

# core KDE frameworks dependencies

"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_extra-cmake-modules
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kconfig
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_breeze-icons
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kcoreaddons
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kwindowsystem
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_solid
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_threadweaver
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_karchive
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kdbusaddons
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_ki18n
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kcrash
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kcodecs
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kguiaddons
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kwidgetsaddons
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kitemviews
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kcompletion

if [[ $DK_QTVERSION == 6 ]] ; then

    "${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kcolorscheme

fi

"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kconfigwidgets
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kiconthemes
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kservice
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kxmlgui
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kbookmarks
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kimageformats

# Extra support for digiKam

# Desktop integration support
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_knotifications
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kjobwidgets
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kio
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_knotifyconfig

# libksane support
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_sonnet
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_ktextwidgets

# Calendar support
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_kcalendarcore

# Breeze style support
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo --target ext_breeze

TerminateScript
