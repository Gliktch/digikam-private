# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2011-11-02
# Description: simple Bash script to replace CRLF by EOL
#
# Copyright (C) 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

#!/bin/bash

for f in `find -iname \*`; do
    if file $f | grep "CRLF"
    then
        echo "Patched EOL of file: $f"
        tr -d '\15\32' < $f > $f.tr
        mv $f.tr $f
    fi
done
