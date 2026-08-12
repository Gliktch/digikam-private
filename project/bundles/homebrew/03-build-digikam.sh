#! /bin/bash

# Script to build digiKam using HomeBrew
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
exec > >(tee ./logs/build-digikam.full.log) 2>&1

#################################################################################################

echo "03-build-digikam.sh : build digiKam using HomeBrew."
echo "---------------------------------------------------"

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

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/opt/qt6/bin:/$INSTALL_PREFIX/opt/bison/bin:$ORIG_PATH

#################################################################################################
# activate the python3 venv
source ${INSTALL_PREFIX}/bin/activate

#################################################################################################
# Install out-dated dependencies

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
      -DINSTALL_ROOT=$INSTALL_PREFIX \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION \
      -DMACOSX_DEPLOYMENT_TARGET=$OSX_MIN_TARGET \
      -DARCH_TARGET=$ARCH_TARGET \
      -DDK_APPLE_PACKAGE_MANAGER=$DK_APPLE_PACKAGE_MANAGER \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
      -Wno-dev

#cmake --build . --config RelWithDebInfo --target ext_exiv2   -- -j$CPU_CORES
#cp $DOWNLOAD_DIR/exiv2_manifest.txt $ORIG_WD/data/
#cmake --build . --config RelWithDebInfo --target ext_lensfun -- -j$CPU_CORES
#cp $DOWNLOAD_DIR/lensfun_manifest.txt $ORIG_WD/data/

#################################################################################################
# Build digiKam in temporary directory and installation

# Clean up previous install (see bug #459276)

cd "$INSTALL_PREFIX"

# This is odd: grep -E under macOS return an error if nothing is found
touch digikam.txt

FILES=$(find . | grep -E '(digikam|showfoto)')

for FILE in $FILES ; do
    if [[ -f ${FILE} || -d ${FILE} ]] ; then
        echo -e "   ==> $INSTALL_PREFIX/${FILE} will be removed from previous install"
        rm -fr $INSTALL_PREFIX/${FILE}
    fi
done

cd $BUILDING_DIR

if [ -d "$DK_BUILDTEMP/digikam-$DK_VERSION" ] ; then

    echo "---------- Updating existing $DK_BUILDTEMP"

    cd "$DK_BUILDTEMP"
    cd digikam-$DK_VERSION

    git reset --hard
    git pull

    rm -fr build
    mkdir -p build

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

fi

echo -e "\n\n"
echo "---------- Configure digiKam $DK_VERSION"

sed -e "s/DIGIKAMSC_COMPILE_PO=OFF/DIGIKAMSC_COMPILE_PO=ON/g"   ./bootstrap.homebrew > ./tmp.homebrew ; mv -f ./tmp.homebrew ./bootstrap.homebrew
sed -e "s/DBUILD_TESTING=ON/DBUILD_TESTING=OFF/g"               ./bootstrap.homebrew > ./tmp.homebrew ; mv -f ./tmp.homebrew ./bootstrap.homebrew
sed -e "s/DENABLE_DBUS=ON/DENABLE_DBUS=OFF/g"                   ./bootstrap.homebrew > ./tmp.homebrew ; mv -f ./tmp.homebrew ./bootstrap.homebrew
sed -e "s/DENABLE_APPSTYLES=OFF/DENABLE_APPSTYLES=ON/g"         ./bootstrap.homebrew > ./tmp.homebrew ; mv -f ./tmp.homebrew ./bootstrap.homebrew

if [[ $DK_QTVERSION = 6 ]] ; then

    sed -e "s/DBUILD_WITH_QT6=OFF/DBUILD_WITH_QT6=ON/g"         ./bootstrap.homebrew > ./tmp.homebrew ; mv -f ./tmp.homebrew ./bootstrap.homebrew

fi

chmod +x ./bootstrap.homebrew

cp -f $ORIG_WD/fixbundledatapath.sh $DK_BUILDTEMP/digikam-$DK_VERSION

./fixbundledatapath.sh

./bootstrap.homebrew "$INSTALL_PREFIX" "Debug" "$ARCH_TARGET" "-Wno-dev"

if [ $? -ne 0 ]; then
    echo "---------- Cannot configure digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

echo -e "\n\n"
echo "---------- Building digiKam $DK_VERSION"

cd build
make -j$CPU_CORES

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

if [[ $DK_QTVERSION == 6 ]] ; then

    # Qt6 install of .../Applications/... binaries is done at the wrong place.

    mkdir -p $INSTALL_PREFIX/$RELOCATE_PREFIX/
    mv $INSTALL_PREFIX/bin/digikam.*  $INSTALL_PREFIX/$RELOCATE_PREFIX/
    mv $INSTALL_PREFIX/bin/showfoto.* $INSTALL_PREFIX/$RELOCATE_PREFIX/

fi

################################################################################################
# Install Extra Plugins

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
       -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
       -DINSTALL_ROOT=$INSTALL_PREFIX \
       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
       -DKP_VERSION=$DK_KP_VERSION \
       -DKA_VERSION=$DK_KA_VERSION \
       -DKDE_VERSION=$DK_KDE_VERSION \
       -DENABLE_QTVERSION=$DK_QTVERSION \
       -DMACOSX_DEPLOYMENT_TARGET=$OSX_MIN_TARGET \
       -DARCH_TARGET=$ARCH_TARGET \
       -DDK_APPLE_PACKAGE_MANAGER=$DK_APPLE_PACKAGE_MANAGER \
       -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
       -Wno-dev

cmake --build . --config RelWithDebInfo --target ext_gmic_qt    -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_mosaicwall -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_flowview   -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
