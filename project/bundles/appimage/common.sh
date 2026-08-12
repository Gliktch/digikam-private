#!/bin/bash

# Common methods for script to build AppImage bundle.
#
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
# Check RAM available in Gb (Linux or MacOS)
ChecksPhyMemory()
{

PHY_MEM=$(LANG=C free|awk '/^Mem:/{print $2}' || sysctl -n hw.memsize)
PHY_MEM=$((PHY_MEM / 1024 / 1024))

echo "Physical Memory (Gb): $PHY_MEM"

}

########################################################################
# Check OS name and version.
CheckSystemReleaseID()
{

OS_NAME=$(awk '/DISTRIB_ID=/' /etc/*-release | sed 's/DISTRIB_ID=//' | sed 's/\"//' | sed 's/\"//' | tr '[:upper:]' '[:lower:]')

if [[ $OS_NAME == "" ]] ; then
    OS_NAME=$(lsb_release -a | grep 'Distributor ID:' | sed 's/Distributor ID://' | xargs | tr '[:upper:]' '[:lower:]')
fi

OS_ARCH=$(uname -m | sed 's/x86_//;s/i[3-6]86/32/')

OS_VERSION=$(awk '/DISTRIB_RELEASE=/' /etc/*-release | sed 's/DISTRIB_RELEASE=//' | sed 's/[.]0/./')

if [[ $OS_VERSION == "" ]] ; then
    OS_VERSION=$(lsb_release -a | grep 'Release:' | sed 's/Release://' | xargs | tr '[:upper:]' '[:lower:]')
fi

echo "Linux: $OS_NAME - $OS_ARCH - $OS_VERSION"

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
# Adjust devel env.for for Linux Host
HostAdjustments()
{

# That's not always set correctly in Mageia 6 at least?
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# Determine which architecture should be built
if [[ "$(arch)" = "i686" || "$(arch)" = "x86_64" ]] ; then
    ARCH=$(arch)
    echo "Architecture is $ARCH"
else
    echo "Architecture could not be determined"
    exit 1
fi

# if the library path doesn't point to our usr/lib, linking will be broken and we won't find all deps either
export LD_LIBRARY_PATH=/usr/lib64/:/usr/lib:/digikam.appdir/usr/lib

}

########################################################################
# Copy dependencies with ldd analysis
# arg1 : original file path to parse.
# arg2 : target path to copy dependencies
CopyReccursiveDependencies()
{

echo "Scan dependencies for $1"

FILES=$(ldd $1 | grep "=>" | awk '{print $3}')

for FILE in $FILES ; do
    if [ -f "$FILE" ] ; then
        cp -u $FILE $2 2> /dev/null || true
#        echo "   ==> $FILE"
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

if [[ ! -f $HOME/.ssh/known_hosts ]] ; then
    touch $HOME/.ssh/known_hosts
fi

for server in $SERVER_LIST; do

    echo "Register $server"

    ssh-keygen -R $server
    ssh-keyscan -H $server >> $HOME/.ssh/known_hosts

done

}

