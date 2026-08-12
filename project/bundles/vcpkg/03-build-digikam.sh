#! /bin/bash

# Script to build digiKam using VCPKG
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

echo "03-build-digikam.sh : build digiKam using VCPKG."
echo "------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksCPUCores
ChecksCmakeBin
RegisterRemoteServers
AppendVCPKGPaths

#################################################################################################

# Paths rules
ORIG_WD="`pwd`"

if [ ! -d $BUILDING_DIR/dk_cmake ] ; then
    mkdir -p $BUILDING_DIR/dk_cmake
fi

cd $BUILDING_DIR/dk_cmake

#################################################################################################
# Check if IcoTool CLI program is installed

if ! which icotool ; then
    echo "IcoTool is not installed"
    echo "See https://www.nongnu.org/icoutils/ for details."
    exit 1
else
    echo "Check IcoTool CLI passed..."
fi

#################################################################################################
# Install out-dated dependencies

if [ ! -d $BUILDING_DIR/dk_cmake ] ; then
    mkdir -p $BUILDING_DIR/dk_cmake
fi

cd $BUILDING_DIR/dk_cmake

rm -rf $BUILDING_DIR/dk_cmake/* || true

"${CMAKE_BIN}" $ORIG_WD/../3rdparty \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=$VCPKG_TRIPLET \
      -DCMAKE_COLOR_MAKEFILE=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_INSTALL_PREFIX=$VCPKG_INSTALL_PREFIX \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DINSTALL_ROOT=$VCPKG_INSTALL_PREFIX \
      -DBUILD_TESTING=OFF \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION \
      -Wno-dev

"${CMAKE_BIN}" --build . --config RelWithDebInfo --target ext_exiv2 --parallel
cp $DOWNLOAD_DIR/exiv2_manifest.txt $ORIG_WD/data/
"${CMAKE_BIN}" --build . --config RelWithDebInfo --target ext_lensfun --parallel
cp $DOWNLOAD_DIR/lensfun_manifest.txt $ORIG_WD/data/

#################################################################################################
# Build digiKam in temporary directory and installation

# Clean up previous install (see bug #459276)

FILES=$(find "$VCPKG_INSTALL_PREFIX" -mindepth 1 \( -name "*digikam*" -o -name "*showfoto*" \)) || true

for FILE in $FILES ; do
    if [[ -f $FILE || -d $FILE ]] ; then
        echo -e "   ==> $FILE will be removed from previous install"
        rm -fr $FILE
    fi
done

cd $BUILDING_DIR

if [ -d "$DK_BUILDTEMP/digikam-$DK_VERSION" ] ; then

    echo "---------- Updating existing $DK_BUILDTEMP"

    cd "$DK_BUILDTEMP"
    cd digikam-$DK_VERSION

    git reset --hard
    git pull

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

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot clone repositories."
        echo "---------- Aborting..."
        exit;
    fi

    cd digikam-$DK_VERSION

    if [ -n "$DK_REVISION" ] ; then

        git checkout $DK_REVISION

    fi

    mkdir build

fi

echo -e "\n\n"
echo "---------- Configure digiKam $DK_VERSION"

sed -e "s/DIGIKAMSC_COMPILE_PO=OFF/DIGIKAMSC_COMPILE_PO=ON/g"   ./bootstrap.vcpkg > ./tmp.vcpkg ; mv -f ./tmp.vcpkg ./bootstrap.vcpkg
sed -e "s/DBUILD_TESTING=ON/DBUILD_TESTING=OFF/g"               ./bootstrap.vcpkg > ./tmp.vcpkg ; mv -f ./tmp.vcpkg ./bootstrap.vcpkg
sed -e "s/DENABLE_DBUS=ON/DENABLE_DBUS=OFF/g"                   ./bootstrap.vcpkg > ./tmp.vcpkg ; mv -f ./tmp.vcpkg ./bootstrap.vcpkg

if [[ $DK_QTVERSION = 5 ]] ; then

    sed -e "s/DBUILD_WITH_QT6=ON/DBUILD_WITH_QT6=OFF/g"         ./bootstrap.vcpkg > ./tmp.vcpkg ; mv -f ./tmp.vcpkg ./bootstrap.vcpkg

fi

chmod +x ./bootstrap.vcpkg

./bootstrap.vcpkg $INSTALL_DIR RelWithDebInfo

if [ $? -ne 0 ]; then
    echo "---------- Cannot configure digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

echo -e "\n\n"
echo "---------- Building digiKam $DK_VERSION"

cd "$DK_BUILDTEMP/digikam-$DK_VERSION/build"
"${CMAKE_BIN}" --build . --parallel --config RelWithDebInfo

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

cd "$DK_BUILDTEMP/digikam-$DK_VERSION/build"
"${CMAKE_BIN}" --install . --config RelWithDebInfo

cd "$ORIG_WD"

if [ $? -ne 0 ]; then
    echo "---------- Cannot install digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

#################################################################################################
# Install Extra Plugins

if [ ! -d $BUILDING_DIR/dk_cmake ] ; then
    mkdir -p $BUILDING_DIR/dk_cmake
fi

cd $BUILDING_DIR/dk_cmake

rm -rf $BUILDING_DIR/dk_cmake/* || true

"${CMAKE_BIN}" $ORIG_WD/../3rdparty \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=$VCPKG_TRIPLET \
      -DCMAKE_COLOR_MAKEFILE=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_INSTALL_PREFIX=$VCPKG_INSTALL_PREFIX \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DINSTALL_ROOT=$VCPKG_INSTALL_PREFIX \
      -DBUILD_TESTING=OFF \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION \
      -Wno-dev

"${CMAKE_BIN}" --build . --config RelWithDebInfo --target ext_mosaicwall --parallel
"${CMAKE_BIN}" --build . --config RelWithDebInfo --target ext_flowview   --parallel
"${CMAKE_BIN}" --build . --config RelWithDebInfo --target ext_gmic_qt    --parallel

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
