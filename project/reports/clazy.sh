#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Clazy analyzer on whole digiKam source code.
# https://github.com/KDE/clazy
# Dependencies : Python BeautifulSoup and SoupSieve at run-time.
#
# If '--nowebupdate' is passed as argument, static analyzer results are not pushed online at
# https://files.kde.org/digikam/reports/ (default yes).
#
# Tested compilation env with Clazy :
#     Kubuntu 24.04.4 LTS + LLVM 19.1.1 + Clazy 1.18 + Qt 6.9.2  + G++ 13.3.0 + CMake 4.1.1
#     Kubuntu 24.04.4 LTS + LLVM 19.1.1 + Clazy 1.18 + Qt 6.10.2 + G++ 13.3.0 + CMake 4.2.3
#     Kubuntu 26.04.0 LTS + LLVM 19.1.7 + Clazy 1.18 + Qt 6.11.1 + G++ 15.2.0 + CMake 4.4.0
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./common.sh

StartScript

# Analyzer configuration.
. ../../.clazy

# Check run-time dependencies

if [ ! -f /opt/clazy/bin/clazy ] ; then

    echo "Clazy static analyzer is not installed in /opt/clazy."
    echo "Please install Clazy from https://github.com/KDE/clazy"
    echo "Aborted..."
    exit -1


else

    echo "Check Clazy static analyzer passed..."

fi

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="report.clazy"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Clazy Static Analyzer task name: $TITLE"

echo "IGNORE DIRS CONFIGURATION: $CLAZY_IGNORE_DIRS"
echo "CHECKERS CONFIGURATION:    $CLAZY_CHECKS"

# Clean up and prepare to scan.

rm -fr $ORIG_WD/$REPORT_DIR
mkdir -p $ORIG_WD/$REPORT_DIR

cd ../..

rm -fr build.clazy
mkdir -p build.clazy
cd build.clazy

#export PATH=$PATH:/opt/clazy/bin

if [[ -d /opt/qt6 ]] ; then

    export BUILD_WITH_QT6=1
    export Qt6_DIR=/opt/qt6
    QTPATHS="/opt/qt6/bin/qtpaths6"
    export CMAKE_BINARY=/opt/qt6/bin/cmake

else

    export BUILD_WITH_QT6=0
    QTPATHS="qtpaths"

fi

export LLVM_VERSION=19

export PATH=/opt/qt6/bin/:/opt/clazy/bin/:$PATH

$CMAKE_BINARY -G "Unix Makefiles" \
      -DCMAKE_CXX_COMPILER=clazy \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DBUILD_WITH_QT6=$BUILD_WITH_QT6 \
      -DBUILD_TESTING=ON \
      -DDIGIKAMSC_CHECKOUT_PO=OFF \
      -DDIGIKAMSC_CHECKOUT_DOC=OFF \
      -DDIGIKAMSC_COMPILE_PO=OFF \
      -DDIGIKAMSC_COMPILE_DOC=OFF \
      -DENABLE_KFILEMETADATASUPPORT=ON \
      -DENABLE_AKONADICONTACTSUPPORT=ON \
      -DENABLE_MYSQLSUPPORT=ON \
      -DENABLE_INTERNALMYSQL=ON \
      -DENABLE_MEDIAPLAYER=ON \
      -DENABLE_QTMULTIMEDIA=ON \
      -DENABLE_DBUS=ON \
      -DENABLE_APPSTYLES=ON \
      -DENABLE_GEOLOCATION=ON \
      -DENABLE_QWEBENGINE=ON \
      -Wno-dev \
      ..

make -j$CPU_CORES 2> ${ORIG_WD}/${REPORT_DIR}/trace.log

cd $ORIG_WD

python3 ./clazy_visualizer.py $ORIG_WD/$REPORT_DIR/trace.log

rm -f $ORIG_WD/$REPORT_DIR/trace.log
mv clazy.html $ORIG_WD/$REPORT_DIR/index.html

if [[ $1 != "--nowebupdate" ]] ; then

    cd $ORIG_WD
    updateOnlineReport "clazy" $REPORT_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_WD

rm -fr ../../build.clazy

TerminateScript
