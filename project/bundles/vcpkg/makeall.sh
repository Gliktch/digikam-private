#!/bin/bash

# Script to run all VCPKG based sub-scripts to build Windows installer.
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

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/win64-Qt6 <<< "rm build-vcpkg.full.log.gz"
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/win64-Qt6 <<< "rm build-extralibs.full.log.gz"

    echo -e "---------- Compress host log files \n"

    gzip -k $ORIG_WD/logs/build-mxe.full.log $ORIG_WD/logs/build-vcpkg.full.log.gz           || true
    gzip -k $ORIG_WD/logs/build-extralibs.full.log $ORIG_WD/logs/build-extralibs.full.log.gz || true

    echo -e "---------- Upload new host logs to files.kde.org repository \n"

    UploadWithRetry $ORIG_WD/logs/build-vcpkg.full.log.gz     $DK_UPLOADURL $DK_UPLOADDIR/build.logs/win64-Qt6 10
    UploadWithRetry $ORIG_WD/logs/build-extralibs.full.log.gz $DK_UPLOADURL $DK_UPLOADDIR/build.logs/win64-Qt6 10

    echo -e "---------- Cleanup local bundle log file archives \n"

    rm -f $ORIG_WD/logs/build-vcpkg.full.log.gz     || true
    rm -f $ORIG_WD/logs/build-extralibs.full.log.gz || true

fi

}

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR
trap HostUploadLogFiles ERR exit

ORIG_WD="`pwd`"

. ./config.sh
. ./common.sh
StartScript

echo "This script will build from scratch the digiKam installer for Windows using VCPKG."
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

if [ -d "$VCPKG_DIR" ] ; then

    if [ "$1" != "-f" ] ; then

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

fi

echo "++++++++++++++++   Build 64 bits Installer   ++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

./01-build-vcpkg.sh
./02-build-extralibs.sh
./update.sh

echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

TerminateScript
