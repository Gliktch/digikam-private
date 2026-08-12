#! /bin/bash

# Script to build extra libraries using HomeBrew
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

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-extralibs.full.log) 2>&1

#################################################################################################

echo "02-build-extralibs.sh : build extra libraries using HomeBrew."
echo "-------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

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

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/opt/qt6/bin:/$INSTALL_PREFIX/opt/bison/bin:/$INSTALL_PREFIX/opt/gperf/bin:$ORIG_PATH

# HomeBrew do not create these sym-links as with other shared libs.

if [ ! -d ${INSTALL_PREFIX}/include/jxl ] ; then
    ln -s ${INSTALL_PREFIX}/opt/jpeg-xl/include/jxl ${INSTALL_PREFIX}/include/jxl
fi

FILES=$(find "${INSTALL_PREFIX}/opt/jpeg-xl/lib/" -name "*.dylib" -depth 1)

for FILE in $FILES ; do
    BASE=$(basename $FILE)

    if [ ! -f ${INSTALL_PREFIX}/lib/${BASE} ] ; then
        ln -s ${FILE} ${INSTALL_PREFIX}/lib/${BASE}
    fi
done

#################################################################################################
# activate the python3 venv
# source ${INSTALL_PREFIX}/bin/activate

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
       -DKP_VERSION=$DK_KP_VERSION \
       -DKDE_VERSION=$DK_KDE_VERSION \
       -DENABLE_QTVERSION=$DK_QTVERSION \
       -DDK_APPLE_PACKAGE_MANAGER=$DK_APPLE_PACKAGE_MANAGER \
       -DARCH_TARGET=$ARCH_TARGET \
       -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
       -Wno-dev

# NOTE: The order to compile each component here is very important.

# core KF6 frameworks dependencies
cmake --build . --config RelWithDebInfo --target ext_extra-cmake-modules -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kconfig             -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_breeze-icons        -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcoreaddons         -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kwindowsystem       -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_solid               -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_threadweaver        -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_karchive            -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kdbusaddons         -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_ki18n               -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcrash              -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcodecs             -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kauth               -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kguiaddons          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kwidgetsaddons      -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kitemviews          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcompletion         -- -j$CPU_CORES

if [[ $DK_QTVERSION == 6 ]] ; then

    cmake --build . --config RelWithDebInfo --target ext_kcolorscheme           -- -j$CPU_CORES

fi

cmake --build . --config RelWithDebInfo --target ext_kconfigwidgets      -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kiconthemes         -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kservice            -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kxmlgui             -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kbookmarks          -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kimageformats       -- -j$CPU_CORES

# Extra support for digiKam

cmake --build . --config RelWithDebInfo --target ext_sonnet              -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_ktextwidgets        -- -j$CPU_CORES

# libksane support
#cmake --build . --config RelWithDebInfo --target ext_libksane            -- -j$CPU_CORES

# Calendar support
cmake --build . --config RelWithDebInfo --target ext_kcalendarcore       -- -j$CPU_CORES

# Breeze style support
cmake --build . --config RelWithDebInfo --target ext_breeze              -- -j$CPU_CORES

# OpenCV with custom options
# cmake --build . --config RelWithDebInfo --target ext_opencv              -- -j$CPU_CORES

#################################################################################################

CopyDebugSymbols

export PATH=$ORIG_PATH

TerminateScript
