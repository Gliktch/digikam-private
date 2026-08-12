#!/bin/bash

# Script to run all Macports based sub-scripts to build MacOS installer.
# Possible option : "-f" to force operations without to ask confirmation to user.
#
# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################
# Function to upload host log files
HostUploadLogFiles()
{

if [[ $DK_UPLOAD = 1 ]] ; then

    echo -e "---------- Cleanup older host logs from files.kde.org repository \n"

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/macos-$TARGET_ARCH <<< "rm build-macports.full.log.gz"
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/macos-$TARGET_ARCH <<< "rm build-extralibs.full.log.gz"

    echo -e "---------- Compress host log files \n"

    gzip -k $ORIG_WD/logs/build-macports.full.log $ORIG_WD/logs/build-macports.full.log.gz   || true
    gzip -k $ORIG_WD/logs/build-extralibs.full.log $ORIG_WD/logs/build-extralibs.full.log.gz || true

    echo -e "---------- Upload new host logs to files.kde.org repository \n"

    rsync -r -v --progress -e ssh $ORIG_WD/logs/build-macports.full.log.gz  $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/macos-$TARGET_ARCH  || true
    rsync -r -v --progress -e ssh $ORIG_WD/logs/build-extralibs.full.log.gz $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/macos-$TARGET_ARCH || true

    echo -e "---------- Cleanup local bundle log file archives \n"

    rm -f $ORIG_WD/logs/build-macports.full.log.gz  || true
    rm -f $ORIG_WD/logs/build-extralibs.full.log.gz || true

fi

}

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR
trap HostUploadLogFiles ERR exit

#################################################################################################
# Pre-processing checks

ORIG_WD="`pwd`"

. ./config.sh
. ./common.sh
StartScript
ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores

echo "This script will build from scratch the digiKam installer for MacOS using Macports."
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

if [ -d "$INSTALL_PREFIX" ] ; then

    if [ "$1" != "-f" ] ; then

        read -p "A previous Macports build already exist and it will be removed. Do you want to continue ? [(c)ontinue/(s)top] " answer

        if echo "$answer" | grep -iq "^s" ; then

            echo "---------- Aborting..."
            exit;

        fi

    fi

    echo "---------- Removing existing Macports build"
    mv $INSTALL_PREFIX $INSTALL_PREFIX.old && rm -fr $INSTALL_PREFIX.old

fi

./01-build-macports.sh
./02-build-extralibs.sh
./update.sh

echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

TerminateScript
