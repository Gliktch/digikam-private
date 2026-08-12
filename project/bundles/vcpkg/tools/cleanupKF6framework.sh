#!/bin/bash

# Script to cleanup installation of KF6 framework from the VCPKG install.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Pre-processing checks

cd ..
. ./common.sh
. ./config.sh

echo $VCPKG_INSTALL_PREFIX

rm -vfr $VCPKG_INSTALL_PREFIX/lib/libKF6*
rm -vfr $VCPKG_INSTALL_PREFIX/share/ECM
rm -vfr $VCPKG_INSTALL_PREFIX/include/KF6
rm -vfr $VCPKG_INSTALL_PREFIX/lib/cmake/KF6*
rm -vfr $VCPKG_INSTALL_PREFIX/include/astro
rm -vfr $VCPKG_INSTALL_PREFIX/include/marble
rm -vfr $VCPKG_INSTALL_PREFIX/data
rm -vfr $VCPKG_INSTALL_PREFIX/lib/marble
rm -vfr $VCPKG_INSTALL_PREFIX/lib/libastro*
rm -vfr $VCPKG_INSTALL_PREFIX/lib/libmarble*
#rm -vfr $VCPKG_PREFIX/lib/cmake/Marble
#rm -vfr $VCPKG_INSTALL_PREFIX/lib/cmake/Astro
