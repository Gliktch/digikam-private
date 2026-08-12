#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause

$EXTRACTRC `find ./core -name \*.rc -o -name \*.ui | grep -v '/tests/'` >> rc.cpp || exit 11
$XGETTEXT rc.cpp `find ./core -name \*.h -o -name \*.cpp | grep -v '/tests/'` `find ./core/app -name \*.h.cmake.in` -o $podir/digikam.pot
