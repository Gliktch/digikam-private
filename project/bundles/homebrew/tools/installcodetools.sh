#!/bin/sh

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
# Updated for Homebrew by Michael Miller <michael underscore miller at dot msn dot com>
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

echo -e "\n"
echo "---------- Installing code analysis tools"
HOMEBREW_NO_AUTO_UPDATE=1 $INSTALL_PREFIX/bin/brew install \
                cppcheck \
                flawfinder \
                lizard-analyzer \
                doxygen

TerminateScript
