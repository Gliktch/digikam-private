#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code

for f in `find \( -name '*.cpp' -o -name '*.h' \)` ; do

    if test -f $f ; then

        CHARSET="$( file -bi "$f"|awk -F "=" '{print $2}')"

        if [ "$CHARSET" != us-ascii ]; then

            echo -e "$f => $CHARSET"

        fi

    else

        echo -e "\nSkipping $f - it's a regular file";

    fi

done
