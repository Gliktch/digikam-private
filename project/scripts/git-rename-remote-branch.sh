#!/bin/bash

# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2016-08-15
# Description: Script to rename remote git branch.
#
# Copyright (C) 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

if [ $# -ne 3 ]; then
    echo "Rationale : Rename a branch on the server without checking it out."
    echo "Usage     : $(basename $0) <remote> <old name> <new name>"
    echo "Example   : $(basename $0) origin master release"
    exit 1
fi

git push $1 $1/$2:refs/heads/$3 :$2
