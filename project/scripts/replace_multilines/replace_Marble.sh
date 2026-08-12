#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Script to replace wrong Marble header with right one
#

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./MarbleO.txt \
      -T ./MarbleN.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./MarbleO.txt \
      -T ./MarbleN.txt \
      -v \
      -i {} \;
