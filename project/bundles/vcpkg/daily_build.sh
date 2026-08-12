#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

while true;
do
    DATE=`date +%H%M`

    if [[ $DATE == "0700" || $DATE == "1100" || $DATE == "1500" || $DATE == "1900"  ]] ; then

        echo "Windows bundle started at $(date)"
        git pull
        ./update.sh
        echo "Windows bundle completed at $(date)"

    fi

    sleep 20s

done
