#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################
# Check if run as root
ChecksRunAsRoot()
{

if [[ $EUID -ne 0 ]]; then
    echo "This script should be run as root using sudo command."
    exit 1
else
    echo "Check run as root passed..."
fi

}

########################################################################
# Check if Xcode Command Line tools are installed
ChecksXCodeCLI()
{

xcode-select --print-path

if [[ $? -ne 0 ]]; then
    echo "XCode CLI tools are not installed"
    echo "See http://www.macports.org/install.php for details."
    exit 1
else
    echo "Check XCode CLI tools passed..."
fi

export MACOSX_DEPLOYMENT_TARGET=$OSX_MIN_TARGET
echo "Target OSX minimal version: $MACOSX_DEPLOYMENT_TARGET"

MACOS_MAJOR=`echo $MACOSX_DEPLOYMENT_TARGET | awk -F '.' '{print $1 "." $2}'| cut -d . -f 1`
MACOS_MINOR=`echo $MACOSX_DEPLOYMENT_TARGET | awk -F '.' '{print $1 "." $2}'| cut -d . -f 2`

if [[ $MACOS_MAJOR -lt 11 && $MACOS_MINOR -lt 9 ]]; then
    export CXXFLAGS=-stdlib=libc++
fi

if [[ ! -d /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_MAJOR.$MACOS_MINOR.sdk ]] ; then

    echo "XCode Target SDK $MACOS_MAJOR.$MACOS_MINOR as minimal version is not installed in /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs"

    if [[ ! -d $DOWNLOAD_DIR/macos-sdk.git ]] ; then

        echo "Downloading archive of SDK from https://github.com/alexey-lysiuk/macos-sdk, please wait..."
        git clone https://github.com/alexey-lysiuk/macos-sdk.git $DOWNLOAD_DIR/macos-sdk.git

    fi

    echo "Copying SDK $MACOS_MAJOR.$MACOS_MINOR into XCode, please wait..."
    cp -R $DOWNLOAD_DIR/macos-sdk.git/MacOSX$MACOS_MAJOR.$MACOS_MINOR.sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/

else

    echo "Check XCode Target SDK minimal version passed..."

fi

# Adjust the property "MinimumSDKVersion" from /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Info.plist
/usr/libexec/PlistBuddy -c "Set MinimumSDKVersion $MACOS_MAJOR.$MACOS_MINOR" /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Info.plist

if [[ $HOST_ARCH = "arm64" ]] ; then

    # Install Metal SDK for QtWebEngine
    xcodebuild -downloadComponent MetalToolchain

fi

}

########################################################################
# Check if Macports is installed
ChecksMacports()
{

which port

if [[ $? -ne 0 ]]; then
    echo "Macports is not installed"
    echo "See http://www.macports.org/install.php for details."
    exit 1
else
    echo "Check Macports passed..."
fi

}

########################################################################
# Check CPU core available (Linux or MacOS)
ChecksCPUCores()
{

CPU_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

if [[ $CPU_CORES -gt 1 ]]; then
    CPU_CORES=$((CPU_CORES-1))
fi

echo "CPU Cores to use : $CPU_CORES"

}

########################################################################
# Performs All Checks
CommonChecks()
{

ChecksRunAsRoot
ChecksXCodeCLI
ChecksMacports

}

########################################################################
# For time execution measurement ; startup
StartScript()
{

BEGIN_SCRIPT=$(date +"%s")

}

########################################################################
# For time execution measurement : shutdown
TerminateScript()
{

TERMIN_SCRIPT=$(date +"%s")
difftimelps=$(($TERMIN_SCRIPT-$BEGIN_SCRIPT))
echo "Elaspsed time for script execution : $(($difftimelps / 3600 )) hours $((($difftimelps % 3600) / 60)) minutes $(($difftimelps % 60)) seconds"

}

########################################################################
# Set strings with detected MacOS info :
#    $MAJOR_OSX_VERSION : detected MacOS major ID (as 10, 11, 12)
#    $MINOR_OSX_VERSION : detected MacOS minor ID (as 7 for 10.7 or 10 for 10.10)
#    $OSX_CODE_NAME     : detected MacOS code name
OsxCodeName()
{

MAJOR_OSX_VERSION=$(sw_vers -productVersion | awk -F '.' '{print $1 "." $2}'| cut -d . -f 1)
MINOR_OSX_VERSION=$(sw_vers -productVersion | awk -F '.' '{print $1 "." $2}'| cut -d . -f 2)

if   [[ $MAJOR_OSX_VERSION == "10" ]] ; then

    if   [[ $MINOR_OSX_VERSION == "15" ]]
        then OSX_CODE_NAME="Catalina"
    elif [[ $MINOR_OSX_VERSION == "14" ]]
        then OSX_CODE_NAME="Mojave"
    elif [[ $MINOR_OSX_VERSION == "13" ]]
        then OSX_CODE_NAME="HighSierra"
    elif [[ $MINOR_OSX_VERSION == "12" ]]
        then OSX_CODE_NAME="Sierra"
    elif [[ $MINOR_OSX_VERSION == "11" ]]
        then OSX_CODE_NAME="ElCapitan"
    elif [[ $MINOR_OSX_VERSION == "10" ]]
        then OSX_CODE_NAME="Yosemite"
    elif [[ $MINOR_OSX_VERSION == "9" ]]
        then OSX_CODE_NAME="Mavericks"
    elif [[ $MINOR_OSX_VERSION == "8" ]]
        then OSX_CODE_NAME="MountainLion"
    elif [[ $MINOR_OSX_VERSION == "7" ]]
        then OSX_CODE_NAME="Lion"
    elif [[ $MINOR_OSX_VERSION == "6" ]]
        then OSX_CODE_NAME="SnowLeopard"
    elif [[ $MINOR_OSX_VERSION == "5" ]]
        then OSX_CODE_NAME="Leopard"
    elif [[ $MINOR_OSX_VERSION == "4" ]]
        then OSX_CODE_NAME="Tiger"
    elif [[ $MINOR_OSX_VERSION == "3" ]]
        then OSX_CODE_NAME="Panther"
    elif [[ $MINOR_OSX_VERSION == "2" ]]
        then OSX_CODE_NAME="Jaguar"
    elif [[ $MINOR_OSX_VERSION == "1" ]]
        then OSX_CODE_NAME="Puma"
    elif [[ $MINOR_OSX_VERSION == "0" ]]
        then OSX_CODE_NAME="Cheetah"
    fi

elif [[ $MAJOR_OSX_VERSION == "11" ]]
    then OSX_CODE_NAME="BigSur"

elif [[ $MAJOR_OSX_VERSION == "12" ]]
    then OSX_CODE_NAME="Monterey"

elif [[ $MAJOR_OSX_VERSION == "13" ]]
    then OSX_CODE_NAME="Ventura"

elif [[ $MAJOR_OSX_VERSION == "14" ]]
    then OSX_CODE_NAME="Sonoma"

elif [[ $MAJOR_OSX_VERSION == "15" ]]
    then OSX_CODE_NAME="Sequoia"

elif [[ $MAJOR_OSX_VERSION == "26" ]]
    then OSX_CODE_NAME="Tahoe"

fi

echo -e "---------- Detected OSX version $MAJOR_OSX_VERSION.$MINOR_OSX_VERSION and code name $OSX_CODE_NAME"

}

#################################################################################################
# Relocate list of binaries files.
# Replace INSTALL_PREFIX by @rpath in library paths dependencies registered in bin file.
# List of bin files to patch is passed as first argument.
RelocatableBinaries()
{

RPATHSTR="@rpath"

FILESLIST=("${!1}")

#echo "Relocatable list: ${FILESLIST[@]}"

for FILE in ${FILESLIST[@]} ; do

    ISMACHO=`file "$FILE" | grep "Mach-O" || true`

    # Do not touch debug extension
    ISDSYM=`file "$FILE" | grep "dSYM" || true`

    if [[ $ISMACHO ]] && [[ ! $ISDSYM ]] ; then

        # For each file from bin list, we replace the absolute path to external dependency with a relative path
        # NOTE: relative path must be resolved in main executable later.

        echo "Relocate $FILE"

        # List all external dependencies starting with INSTALL_PREFIX

        DEPS=$(otool -L $FILE | grep $INSTALL_PREFIX | awk -F ' \\\(' '{print $1}')

        for EXTLIB in $DEPS ; do

            RPATHLIB=${EXTLIB/$INSTALL_PREFIX/$RPATHSTR}
 #           echo "   $EXTLIB ==> $RPATHLIB"
            install_name_tool -change $EXTLIB $RPATHLIB $FILE

        done

    fi

done

}

########################################################################
# Automatically register the remote servers has know hosts
RegisterRemoteServers()
{

SERVER_LIST="\
invent.kde.org \
tinami.kde.org \
"

if [[ ! -f ~/.ssh/known_hosts ]] ; then
    touch ~/.ssh/known_hosts
fi

for server in $SERVER_LIST; do

    echo "Register $server"

    ssh-keygen -R $server
    ssh-keyscan -H $server >> ~/.ssh/known_hosts

done

}
