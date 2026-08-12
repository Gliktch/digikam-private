#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

while true;
do
    DATE=`date +%H%M`

    if [[ $DATE == "1100" ]] ; then

        echo "Linux bundle started at $(date)"
        cd /home/gilles/Documents/7.x/project/bundles/appimage
        git pull
        ./update.sh
        echo "Linux bundle completed at $(date)"

    fi

    sleep 20s

done
