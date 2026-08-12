#!/bin/bash

# ===========================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date:        2016-08-15
# Description: Script to find private objects not hidden.
#
# Copyright (C) 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================

stringA="Q_DECL_HIDDEN"
stringB="class "

# run loop for each file in the directory

for file in `find \( -name '*.cpp' -o -name '*_p.h' \)` ; do

    index=0

    while IFS= read -r line; do

        if [[ $line == *"$stringB"* ]] &&
           [[ $line != *"$stringA"* ]] &&
           [[ $line != *"_EXPORT"*  ]] &&
           [[ $line != *";"         ]] &&
           [[ $line != " * "*       ]] &&
           [[ $line != "// "*       ]] &&
           [[ $line != *"template"* ]]; then

            echo "$file [$index] : $line"

        fi

        index=$(($index+1))

    done < $file

done
