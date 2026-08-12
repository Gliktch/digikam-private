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

COPY_MAIN=1
COPY_EDITOR_PLUGINS=0
COPY_BQM_PLUGINS=0
COPY_GENERIC_PLUGINS=0

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

if [ $COPY_MAIN -eq 1 ]; then
    # copy binaries to bundle location
    cp "$PWD/$LOCAL_BINS/bin/digikam.app/Contents/MacOS/digikam" "$TEMPROOT/$DK_APP_CONTENTS/MacOS"

    # copy main binaries
    rsync -av --include="*/" --include="libdigikam*.dylib" --exclude="*" "$PWD/$LOCAL_BINS/bin/" "$TEMPROOT/$DK_APP_CONTENTS/lib"
fi

# copy Editor, BQM, and Generic plugins
if [ $COPY_EDITOR_PLUGINS -eq 1 ]; then
    rsync -av --include="*/" --include="Editor_*.so" --exclude="*" "$PWD/$LOCAL_BINS/bin/" "$TEMPROOT/$DK_APP_CONTENTS/share/qt/plugins/digikam/editor"
fi
if [ $COPY_BQM_PLUGINS -eq 1 ]; then
    rsync -av --include="*/" --include="BQM_*.so" --exclude="*" "$PWD/$LOCAL_BINS/bin/" "$TEMPROOT/$DK_APP_CONTENTS/share/qt/plugins/digikam/bqm"
fi
if [ $COPY_GENERIC_PLUGINS -eq 1 ]; then
    rsync -av --include="*/" --include="Generic_*.so" --exclude="*" "$PWD/$LOCAL_BINS/bin/" "$TEMPROOT/$DK_APP_CONTENTS/share/qt/plugins/digikam/generic"
fi

if [ $COPY_MAIN -eq 1 ]; then
    # relocate main binaries
    FILES=`find "$TEMPROOT/$DK_APP_CONTENTS/lib" -name "libdigikam*.dylib"`
    for FILE in $FILES ; do
        copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$FILE" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
        eval "$copy_lib"
    done
fi

# relocate Editor binaries
if [ $COPY_EDITOR_PLUGINS -eq 1 ]; then
    echo "Relocating Editor plugins"
    FILES=`find "$TEMPROOT/$DK_APP_CONTENTS/share/qt/plugins/digikam/editor" -name "Editor_*.so"`
    for FILE in $FILES ; do
        copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$FILE" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
        eval "$copy_lib"
    done
fi

# relocate BQM binaries
if [ $COPY_BQM_PLUGINS -eq 1 ]; then
    echo "Relocating BQM plugins"
    FILES=`find "$TEMPROOT/$DK_APP_CONTENTS/share/qt/plugins/digikam/bqm" -name "BQM_*.so"`
    for FILE in $FILES ; do
        copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$FILE" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
        eval "$copy_lib"
    done
fi

# relocate Generic binaries
if [ $COPY_GENERIC_PLUGINS -eq 1 ]; then
    echo "Relocating Generic plugins"
    FILES=`find "$TEMPROOT/$DK_APP_CONTENTS/share/qt/plugins/digikam/generic" -name "Generic_*.so"`
    for FILE in $FILES ; do
        copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$FILE" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
        eval "$copy_lib"
    done
fi

copy_lib="$INSTALL_PREFIX/bin/python3 $PWD/package_lib.py --file="$TEMPROOT/$DK_APP_CONTENTS/MacOS/digikam" --bundle-root=$TEMPROOT/$DK_APP_CONTENTS --homebrew=$INSTALL_PREFIX --processed-cache=use  --found-cache=use --signed-cache=use --update-binary=1 --copy=0 --preserve_rpath=0 --force=1"
eval "$copy_lib"

cp $PWD/../../../core/data/dnnmodels/dnnmodels.conf $TEMPROOT/$DK_APP_CONTENTS/Resources/digikam/dnnmodels

# check if digikam dir exists
if [ ! -d "$HOME/Library/Application Support/digikam" ]; then
    mkdir -p "$HOME/Library/Application Support/digikam/"
fi
cp -r "$PWD/Applications/digiKam.org/digikam.app/Contents/Resources/digikam" "$HOME/Library/Application Support/"

# check if dnnmodels dir exists
if [ ! -d "$HOME/Library/Application Support/digikam/dnnmodels" ]; then
    mkdir -p "$HOME/Library/Application Support/digikam/dnnmodels"
fi
cp $PWD/../../../core/data/dnnmodels/dnnmodels.conf "$HOME/Library/Application Support/digikam/dnnmodels/"

#check if dir exists
if [ ! -d "$HOME/Library/Application Support/digikam/database" ]; then
    mkdir -p "$HOME/Library/Application Support/digikam/database"
fi

#copy dbconfig/xml
cp $PWD/temp.digikam/core/data/database/dbconfig.xml "$HOME/Library/Application Support/digikam/database/dbconfig.xml"

TerminateScript

cd "$ORIG_WD"

date
