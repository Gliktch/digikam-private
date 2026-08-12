#! /bin/bash

# Script to give a console with port CLI tool branched to MAcports bundle repository
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../config.sh
. ../common.sh

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:$ORIG_PATH

CommonChecks

#################################################################################################

port

#################################################################################################

export PATH=$ORIG_PATH
