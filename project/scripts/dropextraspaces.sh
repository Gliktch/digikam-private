#!/bin/bash

# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2011-11-02
# Description: Script to drop trailing whitespace at end line.
#
# Copyright (C) 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

find -name '*.cpp' -print0 | xargs -r0 sed -e 's/[[:blank:]]\+$//' -i
find -name '*.h' -print0 | xargs -r0 sed -e 's/[[:blank:]]\+$//' -i
