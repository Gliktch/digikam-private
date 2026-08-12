#!/bin/bash

# Script to cleanup installation of Qt framework from the host system.
# This script must be run as sudo
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

. ../common.sh
. ../config.sh
ChecksRunAsRoot

rm -vfr $INSTALL_DIR/include/Qt*
rm -vfr $INSTALL_DIR/lib/libQt*
rm -vfr $INSTALL_DIR/lib/cmake/Qt*
rm -vfr $INSTALL_DIR/libexec/Qt*
rm -vfr $INSTALL_DIR/qml
rm -vfr $INSTALL_DIR/resources/qt*
rm -vfr $INSTALL_DIR/translations
rm -vfr $INSTALL_DIR/plugins
rm -vfr $INSTALL_DIR/doc
rm -vfr $INSTALL_DIR/mkspecs/q*.pri
