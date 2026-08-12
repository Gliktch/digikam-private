#!/bin/bash

# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2011-11-02
# Description: Script to replace tabs by 4 spaces.
#
# Copyright (C) 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================


find . -name "*.cpp" -o -name "*.h" | xargs -I {} -t bash -c " expand -t 4 {}  > tmp ; cat tmp > {}"
rm -f tmp
