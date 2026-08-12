#! /bin/bash

# Script to build a bundle VCPKG installation with all digiKam low level dependencies in a dedicated directory.
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
exec > >(tee ./logs/build-vcpkg.full.log) 2>&1

#################################################################################################

echo "01-build-vcpkg.sh : build a bundle VCPKG install with digiKam dependencies."
echo "---------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksCPUCores
RegisterRemoteServers

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

###############################################################################################
# Check if a previous bundle already exist

CONTINUE_INSTALL=0

if [ -d "$VCPKG_DIR" ] ; then

    read -p "$VCPKG_DIR already exist. Do you want to remove it or to continue an aborted previous installation ? [(r)emove/(c)ontinue/(s)top] " answer

    if echo "$answer" | grep -iq "^r" ;then

        echo "---------- Removing existing $VCPKG_BUILDROOT"
        rm -rf "$VCPKG_DIR"

    elif echo "$answer" | grep -iq "^c" ;then

        echo "---------- Continue aborted previous installation in $VCPKG_DIR"
        CONTINUE_INSTALL=1

    else

        echo "---------- Aborting..."
        exit;

    fi

fi

if [[ $CONTINUE_INSTALL == 0 ]]; then

    #################################################################################################
    # Checkout latest VCPKG from github

    git clone https://github.com/Microsoft/vcpkg.git $VCPKG_DIR

    $VCPKG_DIR/bootstrap-vcpkg.bat

fi

#################################################################################################
# Update VCPKG port files

cd $VCPKG_DIR

git pull
$VCPKG_DIR/vcpkg update

#################################################################################################
# Dependencies build and installation

$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libjpeg-turbo[jpeg8]
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install angle
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install mesa[offscreen]
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install openssl
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install pthreads
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install gettext
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install icu
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libpng
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install tiff
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libmysql
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install jasper
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libde265
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libjxl
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install aom
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libavif[aom]
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libheif
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install freeglut
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install brotli
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install liblzma
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install zlib
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install zstd
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install bzip2
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install ffmpeg[ffmpeg,all-nonfree]

if [[ $DK_QTVERSION = 6 ]] ; then

    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtbase[sql-mysql]
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtmultimedia[ffmpeg]
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtnetworkauth
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qttranslations
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtimageformats
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qttools
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtsvg
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtscxml
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtspeech
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtwebchannel
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtpositioning
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qtwebengine
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5compat

else

    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-base[mysqlplugin]
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-multimedia
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-networkauth
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-translations
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-imageformats
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-tools
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-svg
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-scxml
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-speech
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-webchannel
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-webengine
    $VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install qt5-winextras

fi

$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install opencv4[opencl,dnn,openmp,thread]
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install boost-graph
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install boost-uuid
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install lcms
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install eigen3
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install expat
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libxml2
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libxslt
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libical
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install fftw3
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install openexr
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install gperf
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install libsnoretoast
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install hunspell
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install openal-soft
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install curl
$VCPKG_DIR/vcpkg ${VCPKG_COMMON_OPTIONS[@]} install glib --recurse

echo -e "\n"

cd $ORIG_WD
cp -f ../../scripts/create_manifest.sh $VCPKG_DIR
cd $VCPKG_DIR
$VCPKG_DIR/create_manifest.sh $VCPKG_DIR vcpkg
cp $VCPKG_DIR/vcpkg_manifest.txt $ORIG_WD/data/

export PATH=$ORIG_PATH

TerminateScript
