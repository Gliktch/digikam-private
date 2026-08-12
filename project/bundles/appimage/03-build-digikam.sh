#! /bin/bash

# Script to build digiKam under Linux host
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
exec > >(tee ./logs/build-digikam.full.log) 2>&1

#################################################################################################

echo "03-build-digikam.sh : build digiKam."
echo "------------------------------------"

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

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

#################################################################################################
# Install out-dated dependencies

cd $BUILDING_DIR
rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=/usr \
      -DINSTALL_ROOT=/usr \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION

cmake --build . --config RelWithDebInfo --target ext_exiv2         -- -j$CPU_CORES
cp $DOWNLOAD_DIR/exiv2_manifest.txt $ORIG_WD/data/
cmake --build . --config RelWithDebInfo --target ext_lensfun       -- -j$CPU_CORES
cp $DOWNLOAD_DIR/lensfun_manifest.txt $ORIG_WD/data/

#################################################################################################
# Build digiKam in temporary directory and installation

# Clean up previous install (see bug #459276)

FILES=$(find /usr | { grep -E '(digikam|showfoto)' || true; })

for FILE in $FILES ; do
    if [[ -f ${FILE} || -d ${FILE} ]] ; then
        echo -e "   ==> ${FILE} will be removed from previous install"
        rm -fr ${FILE}
    fi
done

if [ -d "$DK_BUILDTEMP/digikam-$DK_VERSION" ] ; then

    echo "---------- Updating existing $DK_BUILDTEMP"

    cd "$DK_BUILDTEMP"
    cd digikam-$DK_VERSION

    git reset --hard
    git pull

    rm -fr build
    mkdir -p build
    cd build

else

    echo "---------- Creating $DK_BUILDTEMP"
    mkdir -p "$DK_BUILDTEMP"

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot create $DK_BUILDTEMP directory."
        echo "---------- Aborting..."
        exit;
    fi

    cd "$DK_BUILDTEMP"
    echo -e "\n\n"
    echo "---------- Downloading digiKam $DK_VERSION"

    git clone --progress --verbose --branch $DK_VERSION --single-branch $DK_GITURL digikam-$DK_VERSION
    cd digikam-$DK_VERSION

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot clone repositories."
        echo "---------- Aborting..."
        exit;
    fi

    mkdir build
    cd build

fi

echo -e "\n\n"
echo "---------- Configure digiKam $DK_VERSION"

#export CC=/usr/bin/clang
#export CXX=/usr/bin/clang++

if [[ $DK_QTVERSION == 5 ]] ; then

    echo "Build digiKam with Qt5"
    BUILD_WITH_QT6=OFF

else

    echo "Build digiKam with Qt6"
    BUILD_WITH_QT6=ON

fi

cmake -G "Unix Makefiles" .. \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DBUILD_TESTING=OFF \
      -DBUILD_WITH_QT6=$BUILD_WITH_QT6 \
      -DBUILD_WITH_CCACHE=ON \
      -DDIGIKAMSC_COMPILE_PO=ON \
      -DDIGIKAMSC_COMPILE_DIGIKAM=ON \
      -DENABLE_KFILEMETADATASUPPORT=OFF \
      -DENABLE_AKONADICONTACTSUPPORT=OFF \
      -DENABLE_MYSQLSUPPORT=ON \
      -DENABLE_INTERNALMYSQL=ON \
      -DENABLE_GEOLOCATION=ON \
      -DENABLE_MEDIAPLAYER=ON \
      -DENABLE_DBUS=OFF \
      -DENABLE_APPSTYLES=ON \
      -DENABLE_QWEBENGINE=ON \
      -DENABLE_FACESENGINE_DNN=ON \
      -DENABLE_KIO=OFF \
      -DENABLE_LEGACY=OFF \
      -Wno-dev

if [ $? -ne 0 ]; then
    echo "---------- Cannot configure digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

echo -e "\n\n"
echo "---------- Building digiKam $DK_VERSION"
make
#make -j$CPU_CORES

if [ $? -ne 0 ]; then
    echo "---------- Cannot compile digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

cat $DK_BUILDTEMP/digikam-$DK_VERSION/build/core/app/utils/digikam_version.h   | grep "digikam_version\[\]" | awk '{print $6}' | tr -d '";'  > $ORIG_WD/data/RELEASEID.txt
cat $DK_BUILDTEMP/digikam-$DK_VERSION/build/core/app/utils/digikam_builddate.h | grep "define BUILD_DATE"   | awk '{print $3}' | tr -d '"\n' > $ORIG_WD/data/BUILDDATE.txt

# Copy manifests for rolling release codes included in digiKam core.
cp  $DK_BUILDTEMP/digikam-$DK_VERSION/core/libs/dimg/filters/transform/lqr/liblqr_manifest.txt $ORIG_WD/data
cp  $DK_BUILDTEMP/digikam-$DK_VERSION/core/libs/rawengine/libraw/libraw_manifest.txt $ORIG_WD/data
cp  $DK_BUILDTEMP/digikam-$DK_VERSION/core/libs/dplugins/webservices/o2/o2_manifest.txt $ORIG_WD/data

echo -e "\n\n"
echo "---------- Installing digiKam $DK_VERSION"
echo -e "\n\n"

make install/fast && cd "$ORIG_WD"

if [ $? -ne 0 ]; then
    echo "---------- Cannot install digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

#################################################################################################
# Install Extra Plugins

echo "Build 3rd-party plugins"

cd $BUILDING_DIR

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=/usr \
      -DINSTALL_ROOT=/usr \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION

cmake --build . --config RelWithDebInfo --target ext_gmic_qt    -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_mosaicwall -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_flowview   -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript

