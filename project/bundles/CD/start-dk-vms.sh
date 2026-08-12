#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

echo "Start DK APPIMAGE Qt6..."
VBoxManage startvm "DK APPIMAGE Qt6"

echo "Start DK APPIMAGE Qt5..."
VBoxManage startvm "DK APPIMAGE Qt5"

echo "Start DK WINDOWS 11 QT6..."
VBoxManage startvm "DK WINDOWS 11 QT6"

echo "Start DK WINDOWS 10 QT5..."
VBoxManage startvm "DK WINDOWS 10 QT5"

echo "All VM started."

exit 0
