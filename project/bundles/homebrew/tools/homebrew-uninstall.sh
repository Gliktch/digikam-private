#!/bin/bash

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
# Updated for Homebrew by Michael Miller <michael underscore miller at dot msn dot com>
# This script must be run as sudo
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Pre-processing checks

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../config.sh
. ../common.sh
StartScript
ChecksRunAsRoot

ORIG_WD="`pwd`"
cd ..
UNINSTALL_SCRIPT="uninstall-custom-brew.sh"
. ./config.sh
echo $UNINSTALL_SCRIPT
echo $INSTALL_PREFIX
curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/uninstall.sh >> $UNINSTALL_SCRIPT
chmod +x $UNINSTALL_SCRIPT
. ./$UNINSTALL_SCRIPT -f --path=$INSTALL_PREFIX
rm $UNINSTALL_SCRIPT
sudo rm -rf $INSTALL_PREFIX
cd $ORIG_WD

TerminateScript
