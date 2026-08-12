#!/bin/sh

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
# Updated for Homebrew by Michael Miller <michael underscore miller at dot msn dot com>
# This script must be run as sudo
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Pre-processing checks

# Ask to run as root - Not needed for Homebrew
# (( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../config.sh
. ../common.sh
StartScript

# Update Homebrew binary

echo -e "\n"
echo "---------- Updating HomeBrew"
$INSTALL_PREFIX/bin/brew -v update

# Update all already install packages

$INSTALL_PREFIX/bin/brew -v upgrade
echo -e "\n"

TerminateScript
