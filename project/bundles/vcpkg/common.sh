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
# Copy dependencies with dumpbin analysis
# arg1 : dumpbin program full path.
# arg2 : original file path to parse.
# arg3 : target path to copy dependencies.
# arg4 : dependencies path (only copy shared libs from this path).
CopyReccursiveDependencies()
{

echo "Scan dependencies for $2"

FILES=$("$1" -DEPENDENTS "$2" | grep ".dll" | awk '{print $1}')
#echo "deps: $FILES"
for FILE in $FILES ; do
    if [[ -f "$4/$FILE"  && ! -f  "$3/$FILE" ]] ; then
        cp -u "$4/$FILE" "$3" 2> /dev/null || true
#        echo "   ==> $4/$FILE"
        CopyReccursiveDependencies "$1" "$4/$FILE" "$3" "$4"
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

########################################################################
# Append paths to the wanted VCPKG binary tools
AppendVCPKGPaths()
{

ORIG_PATH="$PATH"

export PATH="\
$PATH:\
/c/bison:\
/c/icoutils/bin:\
$VCPKG_INSTALL_PREFIX/tools/gettext/bin:\
$VCPKG_INSTALL_PREFIX/tools/gperf:\
$VCPKG_INSTALL_PREFIX/tools/curl:\
$VCPKG_INSTALL_PREFIX/tools/pkgconf:\
$VCPKG_INSTALL_PREFIX/bin:\
$DOWNLOAD_DIR/tools/python/python-3.14.2-x64-1:\
$DOWNLOAD_DIR/tools/ninja-1.13.2-windows:\
"

if [[ $DK_QTVERSION = 6 ]] ; then

    export PATH="\
    $PATH:\
    $VCPKG_INSTALL_PREFIX/tools/Qt6/bin\
    "

else

    export PATH="\
    $PATH:\
    $VCPKG_INSTALL_PREFIX/tools/qt5/bin\
    "

fi

echo "PATH=$PATH"

export PKG_CONFIG_PATH=$VCPKG_INSTALL_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH

}

########################################################################
# Upload a large file to a remote server with retry if failure
# arg1 : local file path to upload.
# arg2 : remote ssh url.
# arg3 : remote directory.
# arg4 : pause between retry.
UploadWithRetry()
{

# Disable errors exception handling
set +e

MAX_RETRIES=10
i=0
RC=1

while [[ $RC -ne 0 ]] ; do

    i=$(($i+1))

    if [ $i -eq $MAX_RETRIES ] ; then
        echo -e "Hit maximum number of retries, giving up."
        exit -1
    fi

    sleep $4
    echo -e "Try $i/$MAX_RETRIES :: rsync -r -v --progress -e ssh $1 $2:$3"
    bash -c "rsync -r -v --progress -e ssh $1 $2:$3"
    RC=$?
    echo "rsync return code: $RC"

done

# Re-enable halt and catch errors
set -eE

}

ChecksCmakeBin()
{

# Check if the Cmake CLI tool is installed with Visual Studio.

CMAKE_BIN="`find "/c/Program Files/Microsoft Visual Studio/" -name "cmake.exe" -type f -executable`"
echo "$CMAKE_BIN"

if [ ! -f "$CMAKE_BIN" ] ; then
    echo "CMake CLI tool is not installed"
    echo "Install VSCommunity C++ CMake tools component."
    exit 1
else
    echo "Check CMake CLI tools passed..."
fi

}
