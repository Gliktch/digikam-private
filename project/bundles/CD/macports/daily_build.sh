#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

while true;
do
    DATE=`date +%H%M`

    if [[ $DATE == "1300" ]] ; then

        echo "macOS bundle started at $(date)"
        cd /Users/gilles/Documents/7.x/project/bundles/macports
        git pull
        ./update.sh
        echo "macOS bundle completed at $(date)"

    fi

    sleep 20

done
