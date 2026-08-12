#! /bin/bash

# Script to bundle data using previously-built digiKam installation.
# and create a PKG file with Packages application (http://s.sudre.free.fr/Software/Packages/about.html)
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015      by Shanti, <listaccount at revenant dot org>
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2024      by Michael Miller, <michael underscore miller at msn dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# This script is used to move binaries from a local build directory (I use temp.digikam) to the bundle
# directory for testing.  This can be run only after steps 01-04 have been completed.
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

ORIG_WD="`pwd`"

cd ..
. ./config.sh
. ./common.sh

StartScript

# Directory where this script is located (default - current directory)
BUILDDIR="$PWD"

# Directory where Packages project files are located
PROJECTDIR="$BUILDDIR/installer"

# Staging area where files to be packaged will be copied
TEMPROOT="$BUILDDIR/$RELOCATE_PREFIX"

DK_APP_CONTENTS="digikam.app/Contents"

# local binary directory
LOCAL_BINS="temp.digikam"

cp $PWD/../../../core/data//dnnmodels/dnnmodels.conf $TEMPROOT/$DK_APP_CONTENTS/Resources/digikam/dnnmodels
cp $PWD/../../../core/data//dnnmodels/dnnmodels.conf "$HOME/Library/Application Support/digikam/dnnmodels/"

TerminateScript

cd "$ORIG_WD"

date
