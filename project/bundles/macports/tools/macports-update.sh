#!/bin/sh

# SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
# This script must be run as sudo
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Pre-processing checks

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../common.sh
StartScript
ChecksRunAsRoot

# Update Macports binary

port selfupdate

# Update all already install packages

port upgrade outdated

TerminateScript
