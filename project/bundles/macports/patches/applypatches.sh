#! /bin/bash

# Script to apply Macports port patches recursively
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ../config.sh

FILES=$(find . -iname "*.patch")

for patchfile in $FILES ; do

    pfile="$(basename "$patchfile")"
    pdir="$(dirname "$patchfile")"

    if [[ $pfile == "Portfile.patch" ]] ; then

        echo "> Apply patch: $patchfile"
        patch -u $INSTALL_PREFIX/var/macports/sources/rsync.macports.org/macports/release/tarballs/ports/$pdir/Portfile -i $patchfile

    else

        echo "> Copy patch: $patchfile"
        cp -v $patchfile $INSTALL_PREFIX/var/macports/sources/rsync.macports.org/macports/release/tarballs/ports/$pdir

    fi

done
