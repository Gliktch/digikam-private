#!/bin/bash

# Script to attach AppImage main program to a remote GDB instance.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Check if run as root.
# Necessary if the target process runs with a different user or if SELinux/AppArmor restrictions are on.

if [[ $EUID -ne 0 ]] ; then
    echo "This script should be run as root using sudo command."
    exit 1
else
    echo "Check run as root passed..."
fi

# ---

# The application process ID to handle.

pid=$(pidof digikam)

if [ -z "$pid" ] ; then
    echo "Error: Cannot find the running digiKam instance..."
    exit 1
fi

echo "pid=$pid"

# The corresponding binary application path.

binary=$(readlink -f "/proc/$pid/exe")

if [ ! -f "$binary" ]; then
    echo "Error : Cannot find the digiKam instance path..."
    exit 1
fi

echo "binary=$binary"

# The root application path.

dir=$(dirname "$binary")
echo "dir=$dir"

# Remote GDB instance call.
# NOTE: the gdb trace will be stored in gdb.txt file.

gdb "$binary" \
    -p $pid \
    -batch \
    -ex "set sysroot $dir" \
    -ex "set solib-search-path $dir/../lib/" \
    -ex "set pagination off" \
    -ex "set logging enabled on" \
    -ex "catch throw" \
    -ex "continue" \
    -ex "quit" \
    2>/dev/null

chown 666 ./gdb.txt

