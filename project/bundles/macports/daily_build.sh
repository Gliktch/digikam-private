#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

while true;
do
    DATE=`date +%H%M`

    if [[ $DATE == "0100" || $DATE == "0300" || $DATE == "0500" ]] ; then

        echo "macOS bundle started at $(date)"
        git pull
        ./update.sh
        echo "macOS bundle completed at $(date)"

    fi

    sleep 20

done
