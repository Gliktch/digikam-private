#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

echo "Save DK APPIMAGE Qt6..."
VBoxManage controlvm "DK APPIMAGE Qt6" savestate

echo "Save DK APPIMAGE Qt5..."
VBoxManage controlvm "DK APPIMAGE Qt5" savestate

echo "Save DK WINDOWS 11 QT6..."
VBoxManage controlvm "DK WINDOWS 11 QT6" savestate

echo "Save DK WINDOWS 10 QT5..."
VBoxManage controlvm "DK WINDOWS 10 QT5" savestate

echo "All VM saved. Halting system now..."

systemctl halt

exit 0
