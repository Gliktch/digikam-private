#!/bin/bash

# Script to cleanup installation of KDE framework from the Macports install.
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

echo $INSTALL_PREFIX

rm -vfr $INSTALL_PREFIX/lib/libKF${DK_QTVERSION}*
rm -vfr $INSTALL_PREFIX/share/ECM
rm -vfr $INSTALL_PREFIX/share/KF${DK_QTVERSION}
rm -vfr $INSTALL_PREFIX/include/KF${DK_QTVERSION}
rm -vfr $INSTALL_PREFIX/lib/cmake/KF${DK_QTVERSION}*
rm -vfr $INSTALL_PREFIX/lib/libexec/KF${DK_QTVERSION}
rm -vfr $INSTALL_PREFIX/lib/plugins/KF${DK_QTVERSION}
rm -vfr $INSTALL_PREFIX/lib/plugins/imageformats/kimg*
rm -vfr $INSTALL_PREFIX/lib/plugins/kiconthemes${DK_QTVERSION}
rm -vfr $INSTALL_PREFIX/lib/plugins/styles
rm -vfr $INSTALL_PREFIX/lib/pkgconfig/KF${DK_QTVERSION}*
