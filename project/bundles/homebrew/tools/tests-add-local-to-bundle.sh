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

# copy binaries to bundle location
cp "$PWD/$LOCAL_BINS/bin/digikam.app/Contents/MacOS/digikam" "$TEMPROOT/$DK_APP_CONTENTS/MacOS"
cp "$PWD/$LOCAL_BINS/bin/identityprovider_utest" "$TEMPROOT/$DK_APP_CONTENTS/MacOS"
cp "$PWD/$LOCAL_BINS/bin/faceclassifier_utest" "$TEMPROOT/$DK_APP_CONTENTS/MacOS"

rsync -av --include="*/" --include="libdigikam*.dylib" --exclude="*" "$PWD/$LOCAL_BINS/bin/" "$TEMPROOT/$DK_APP_CONTENTS/lib"

# copy QtTest library
mkdir -p "$TEMPROOT/$DK_APP_CONTENTS/lib/opt/qt/lib"
cp -r /opt/hb-digikam.org.arm64/Cellar/qt/6.7.3/lib/QtTest.framework "$TEMPROOT/$DK_APP_CONTENTS/lib/opt/qt/lib/"

# relocate binaries
FILES=`find "$TEMPROOT/$DK_APP_CONTENTS/lib" -name "libdigikam*.dylib"`
for FILE in $FILES ; do
    copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$FILE" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
    eval "$copy_lib"
done

copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$TEMPROOT/$DK_APP_CONTENTS/MacOS/digikam" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
eval "$copy_lib"

copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$TEMPROOT/$DK_APP_CONTENTS/MacOS/identityprovider_utest" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
eval "$copy_lib"

copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$TEMPROOT/$DK_APP_CONTENTS/MacOS/faceclassifier_utest" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
eval "$copy_lib"

cp $PWD/../../../core/data//dnnmodels/dnnmodels.conf $TEMPROOT/$DK_APP_CONTENTS/Resources/digikam/dnnmodels
cp $PWD/../../../core/data//dnnmodels/dnnmodels.conf "$HOME/Library/Application Support/digikam/dnnmodels/"

TerminateScript

cd "$ORIG_WD"

date
