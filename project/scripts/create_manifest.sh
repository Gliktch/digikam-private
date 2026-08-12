#!/bin/bash

# Script to create a manifest text file with git revision.
#
# Arguments : $1 : download directory.
#             $2 : component name
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -e
set -C

if [ "$1" == "" ]; then
    DOWNLOAD_DIR=$PWD
else
    DOWNLOAD_DIR=$1
fi

echo "Download directory: $DOWNLOAD_DIR"

# --- List git revisions for all sub-modules

GITREV_LST=$DOWNLOAD_DIR/$2_manifest.txt

rm -f $GITREV_LST
currentDate=`date +"%Y-%m-%d"`
upperName=`echo $2 | awk '{print toupper($0)}'`

echo "+$upperName Snapshoot $currentDate" > $GITREV_LST

echo "$2:$(cd $PWD && git rev-parse HEAD)" >> $GITREV_LST

cat $GITREV_LST
