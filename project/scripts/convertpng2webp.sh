# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2011-11-02
# Description: simple Bash script to convert PNG file to WEBP using IM
#
# Copyright (C) 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

#!/bin/bash

for f in `find -iname \*.png`; do
    echo "PNG to WEBP: $f ${f%.*}.webp"
    convert $f ${f%.*}.webp
done
