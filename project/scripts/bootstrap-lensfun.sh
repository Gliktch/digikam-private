#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code

#export VERBOSE=1

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export Options='-DBUILD_STATIC=OFF \
                -DBUILD_LENSTOOL=OFF \
                -DBUILD_FOR_SSE=OFF \
                -DBUILD_FOR_SSE2=OFF \
                -DBUILD_DOC=OFF \
                -DINSTALL_PYTHON_MODULE=OFF \
                -DINSTALL_HELPER_SCRIPTS=OFF \
                -DBUILD_TESTS=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $Options \
      ..

