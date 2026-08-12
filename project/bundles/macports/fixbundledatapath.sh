#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################
# Fix QStandardPaths problem with non bundle like place to sore data under OSX.

LC_CTYPE=C find . -name "*.cpp" -type f -exec sed -i '' -e 's/GenericDataLocation/AppDataLocation/g' {} \;

