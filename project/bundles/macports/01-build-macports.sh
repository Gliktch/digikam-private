#! /bin/bash

# Script to build a bundle Macports installation with all digiKam dependencies in a dedicated directory
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015      by Shanti <listaccount at revenant dot org>
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
exec > >(tee ./logs/build-macports.full.log) 2>&1

#################################################################################################

echo "01-build-macports.sh : build a bundle Macports install with digiKam dependencies."
echo "---------------------------------------------------------------------------------"

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

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/libexec/qt$DK_QTVERSION/bin:$ORIG_PATH:/$INSTALL_PREFIX/libexec/gnubin
echo "PATH=$PATH"
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH/$INSTALL_PREFIX
echo "DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH"

#################################################################################################
# Check if /opt exists and standard Macports install path

if [ ! -d "/opt" ] ; then

    echo "---------- /opt do not exist, creating"

    mkdir "/opt"

    if [ $? -ne 0 ] ; then

        echo "---------- Cannot create /opt directory."
        echo "---------- Aborting..."
        exit;

    fi

fi

#################################################################################################
# Check if a previous bundle already exist

CONTINUE_INSTALL=0

if [ -d "$INSTALL_PREFIX" ] ; then

    read -p "$INSTALL_PREFIX already exist. Do you want to remove it or to continue an aborted previous installation ? [(r)emove/(c)ontinue/(s)top] " answer

    if echo "$answer" | grep -iq "^r" ;then

        echo "---------- Removing existing $INSTALL_PREFIX"
        mv $INSTALL_PREFIX $INSTALL_PREFIX.old || true
        rm -rf $INSTALL_PREFIX.old || true

    elif echo "$answer" | grep -iq "^c" ;then

        echo "---------- Continue aborted previous installation in $INSTALL_PREFIX"
        CONTINUE_INSTALL=1

    else

        echo "---------- Aborting..."
        exit;

    fi

fi

if [[ $CONTINUE_INSTALL == 0 ]]; then

    #################################################################################################
    # Target directory creation

    echo "---------- Creating $INSTALL_PREFIX"

    mkdir -p "$INSTALL_PREFIX"

    if [ $? -ne 0 ] ; then

         echo "---------- Cannot create target install directory $INSTALL_PREFIX"
         echo "---------- Aborting..."
         exit;

    fi

    #################################################################################################
    # Check latest Macports version available if necessary

    if [ -z $MP_VERSION ] ; then

        MP_LASTEST_VER=$(curl $MP_URL | \
            egrep -o 'href="MacPorts-[0-9]+\.[0-9]+\.[0-9]+' | \
            sed 's/^href="MacPorts-//' | \
            sort -t. -rn -k1,1 -k2,2 -k3,3 | head -1)

        if [ -z $MP_LASTEST_VER ] ; then

            echo "---------- Cannot check the latest Macports version from $MP_URL"
            echo "---------- Aborting..."
            exit;

        fi

        echo "---------- Detected latest Macports version : $MP_LASTEST_VER"

        MP_VERSION=$MP_LASTEST_VER

    fi

    #################################################################################################
    # Build Macports in temporary directory and installation

    if [ -d "$MP_BUILDTEMP" ] ; then

        echo "---------- Removing existing $MP_BUILDTEMP"
        rm -rf "$MP_BUILDTEMP"

    fi

    echo "---------- Creating $MP_BUILDTEMP"
    mkdir "$MP_BUILDTEMP"

    if [ $? -ne 0 ] ; then

        echo "---------- Cannot create temporary directory $MP_BUILDTEMP to compile Macports"
        echo "---------- Aborting..."
        exit;

    fi

    cd "$MP_BUILDTEMP"
    echo -e "\n\n"

    echo "---------- Downloading MacPorts $MP_VERSION"
    curl -o "MacPorts-$MP_VERSION.tar.bz2" "$MP_URL/MacPorts-$MP_VERSION.tar.bz2"
    tar jxvf MacPorts-$MP_VERSION.tar.bz2
    cd MacPorts-$MP_VERSION
    echo -e "\n\n"

    echo "---------- Configuring MacPorts"

    ./configure --prefix="$INSTALL_PREFIX"                             \
                --with-applications-dir="$INSTALL_PREFIX/Applications" \
                --with-no-root-privileges                              \
                --with-install-user="$(id -n -u)"                      \
                --with-install-group="$(id -n -g)"

    echo -e "\n\n"

    echo "---------- Building MacPorts"
    make -j$CPU_CORES
    echo -e "\n\n"

    echo "---------- Installing MacPorts"
    echo -e "\n\n"
    make install && cd "$ORIG_WD" && rm -rf "$MP_BUILDTEMP"

    cat << EOF >> "$INSTALL_PREFIX/etc/macports/macports.conf"
+no_root -startupitem
startupitem_type none
startupitem_install no
macosx_deployment_target $OSX_MIN_TARGET
build_arch $HOST_ARCH
EOF

    if [[ $HOST_ARCH = "arm64" && $MP_UNIVERSAL == 1 ]] ; then

        # Build with both architectures embedded (x86 and ARM) for Apple Silicon target

        cat << EOF >> "$INSTALL_PREFIX/etc/macports/variants.conf"
+universal
EOF

    fi

fi

#################################################################################################
# Macports update

echo -e "\n"
echo "---------- Updating MacPorts"
port -v selfupdate

#cd patches
#./applypatches.sh
cd $ORIG_WD

if [[ $CONTINUE_INSTALL == 0 ]]; then

#    port -v upgrade outdated
    echo -e "\n"

    #echo "---------- Modifying net-snmp portfile to install when not root"
    #sed -e "/install.asroot/ s|yes|no|" -i ".orig" "`port file net-snmp`"

fi

#################################################################################################

# Create the build dir for the 3rdparty deps compiled with CMake

if [ ! -d $BUILDING_DIR ] ; then

    mkdir $BUILDING_DIR

fi

if [ ! -d $DOWNLOAD_DIR ] ; then

    mkdir $DOWNLOAD_DIR

fi

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

#################################################################################################
# Dependencies build and installation

echo -e "\n"
echo "---------- Building digiKam dependencies with Macports"

# With MacOS less than El Capitan, we need a more recent Clang compiler than one provided by XCode.
# This only concern x86_64 architecture.

if [[ $MAJOR_OSX_VERSION -lt 11 && $MINOR_OSX_VERSION -lt 10 ]]; then

    echo "---------- Install more recent Clang compiler from Macports for specific ports"
    port install clang_select
    port install clang-3.4
    port select --set clang mp-clang-3.4

fi

echo -e "\n"

if [[ $HOST_ARCH = "x86_64" ]] ; then

    port install ld64 +ld64_xcode

fi

echo -e "MariaDB Variant=$MP_MARIADB_VARIANT\n"

# To fix broken ports with m4 tool detection.

port install m4

if [ ! -f $INSTALL_PREFIX/bin/m4 ] ; then

    cp -f $INSTALL_PREFIX/libexec/gnubin/m4 $INSTALL_PREFIX/bin/

fi


port install git
port install cctools +xcode
port install cmake
port install libpng
port install jpeg
port install tiff
port install eigen3
port install gettext
port install jasper
port install lcms2
port install expat
port install libxml2
port install libxslt
port install bison
port install x265
port install libde265
port install libfdk-aac
port install lame
port install speex
port install libtheora
port install xvid
port install libass
port install opus-tools
port install libvpx
port install aom
port install libusb
port install libgphoto2
port install wget +ssl
port install libical

port install libheif +x265 -tests -rav1e -kvazaar
port install ffmpeg8 +gpl2 +nonfree

if [ ! -f $INSTALL_PREFIX/bin/ffmpeg ] ; then

    # Need to push ffmpeg8 from $INSTALL_PREFIX/libexec/ffmpeg8 to $INSTALL_PREFIX
    cp -r $INSTALL_PREFIX/libexec/ffmpeg8/include $INSTALL_PREFIX
    cp -r $INSTALL_PREFIX/libexec/ffmpeg8/lib     $INSTALL_PREFIX
    mv $INSTALL_PREFIX/bin/ffmpeg8 $INSTALL_PREFIX/bin/ffmpeg

fi

port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtbase -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtdeclarative -examples -tests

if [[ $DK_QTVERSION = 5 ]] ; then

    port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtmacextras -examples -tests
    port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtquickcontrols -examples -tests
    port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtxmlpatterns -examples -tests
fi

port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtscxml -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtsvg -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qttools -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qttranslations
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtimageformats -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtmultimedia -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtnetworkauth -examples -tests
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-sqlite-plugin
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-mysql-plugin $MP_MARIADB_VARIANT
port install --allow-failing qt$DK_QTVERSION$MP_QTSUBVERSION-qtwebengine -examples -tests

port install boost

cmake $ORIG_WD/../3rdparty \
       -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
       -DINSTALL_ROOT=$INSTALL_PREFIX \
       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
       -DKA_VERSION=$DK_KA_VERSION \
       -DKP_VERSION=$DK_KP_VERSION \
       -DKDE_VERSION=$DK_KDE_VERSION \
       -DENABLE_QTVERSION=$DK_QTVERSION \
       -DDK_APPLE_PACKAGE_MANAGER=$DK_APPLE_PACKAGE_MANAGER \
       -DARCH_TARGET=$TARGET_ARCH \
       -Wno-dev

cmake --build . --config RelWithDebInfo --target ext_opencv      -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_imagemagick -- -j$CPU_CORES

port install libjxl
port install libavif -docs -tests

# for rll.py script.
port install python27
port select --set python2 python27

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
