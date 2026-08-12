#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Script to show all KF5 dependencies by macro includes scan

grep -r "#include <k" ../../core/*          | \
    grep -v "#include <klocalizedstring.h>" | \
    grep -v "config"                        | \
    grep -v "kernel"                        | \
    grep -v "kvm"                           | \
    grep -v "kstat"                         | \
    grep -v "keychain"                      | \
    grep -v "scankdedep.sh"
