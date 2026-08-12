#!/bin/bash

# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2016-08-15
# Description: Script to find Exiv2 tags strings.
#
# Copyright (C) 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

find ../../ -name "*.cpp" -type f | xargs grep -n "\"Exif\." | awk -F'[\"\"]' '{print $2}' |sort -u |cut -d' ' -f 1 | grep "Exif."
find ../../ -name "*.cpp" -type f | xargs grep -n "\"Iptc\." | awk -F'[\"\"]' '{print $2}' |sort -u |cut -d'=' -f 1 | grep "Iptc."
find ../../ -name "*.cpp" -type f | xargs grep -n "\"Xmp\." | awk -F'[\"\"]' '{print $2}' |sort -u |cut -d'=' -f 1 | grep "Xmp."
