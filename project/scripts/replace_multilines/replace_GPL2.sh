#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Script to replace GPL2+ header with SDPX-GPL2
#

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./GPL2_1.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./GPL2_1.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./GPL2_2.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./GPL2_2.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./GPL2_3.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./GPL2_3.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./GPL2_4.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./GPL2_4.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./GPL2_5.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./GPL2_5.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.cpp" \
    -exec ./replace.py \
      -F ./GPL2_6.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;

find .. -type f -name "*.h" \
    -exec ./replace.py \
      -F ./GPL2_6.txt \
      -T ./SPDX-GPL2.txt \
      -v \
      -i {} \;



