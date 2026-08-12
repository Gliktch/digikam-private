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

export Options='-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
                -DOPENSSL_BUILD_VERBOSE=ON \
                -DOPENSSL_ENABLE_PARALLEL=ON \
                -DOPENSSL_INSTALL=OFF \
                -DOPENSSL_INSTALL_CERT=OFF \
                -DOPENSSL_TEST=OFF \
                -DOPENSSL_USE_CCACHE=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $Options \
      ..

