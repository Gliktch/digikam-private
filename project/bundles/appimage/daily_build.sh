#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

while true;
do
    DATE=`date +%H%M`

    if [[ $DATE == "0800" || $DATE == "1200" || $DATE == "1600" || $DATE == "2000"  ]] ; then

        echo "Linux bundle started at $(date)"
        git pull
        ./update.sh
        echo "Linux bundle completed at $(date)"

    fi

    sleep 20s

done
