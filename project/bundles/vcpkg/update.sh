#!/bin/bash

# Script to update digiKam Windows installers.
#
# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################
# Function to upload bundle log files
BundleUploadLogFiles()
{

if [[ $DK_UPLOAD = 1 ]] ; then

    echo -e "---------- Cleanup older bundle logs from files.kde.org repository \n"

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/win64-Qt$DK_QTVERSION <<< "rm build-digikam.full.log.gz"
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/win64-Qt$DK_QTVERSION <<< "rm build-installer.full.log.gz"

    echo -e "---------- Compress bundle log files \n"

    gzip -k $ORIG_WD/logs/build-digikam.full.log   $ORIG_WD/logs/build-digikam.full.log.gz   || true
    gzip -k $ORIG_WD/logs/build-installer.full.log $ORIG_WD/logs/build-installer.full.log.gz || true

    echo -e "---------- Upload new bundle logs to files.kde.org repository \n"

    UploadWithRetry $ORIG_WD/logs/build-digikam.full.log.gz   $DK_UPLOADURL $DK_UPLOADDIR/build.logs/win64-Qt$DK_QTVERSION 10
    UploadWithRetry $ORIG_WD/logs/build-installer.full.log.gz $DK_UPLOADURL $DK_UPLOADDIR/build.logs/win64-Qt$DK_QTVERSION 10

    echo -e "---------- Cleanup local bundle log file archives \n"

    rm -f $ORIG_WD/logs/build-digikam.full.log.gz   || true
    rm -f $ORIG_WD/logs/build-installer.full.log.gz || true

fi

}

########################################################################

. ./config.sh
. ./common.sh

# Halt and catch errors. Upload log files if necessary at script error or exit.
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR
trap BundleUploadLogFiles ERR exit

StartScript

ORIG_WD="`pwd`"

echo "++++++++++++++++   Build 64 bits Installer   ++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

./03-build-digikam.sh

sed -e "s/DK_DEBUG=1/DK_DEBUG=0/g"           ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

./04-build-installer.sh

sed -e "s/DK_DEBUG=0/DK_DEBUG=1/g"           ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

./04-build-installer.sh

sed -e "s/DK_DEBUG=1/DK_DEBUG=0/g"           ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

TerminateScript
