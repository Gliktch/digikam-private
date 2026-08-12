#!/bin/bash

# SPDX-FileCopyrightText: 2008-2025 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

export Options='-DCLAZY_AST_MATCHERS_CRASH_WORKAROUND=ON \
                -DLINK_CLAZY_TO_LLVM=ON \
                -DAPPIMAGE_HACK=OFF \
                -DCLAZY_MAN_PAGE=OFF \
                -DCMAKE_EXE_LINKER_FLAGS="-lstdc++" \
                -DCMAKE_SHARED_LINKER_FLAGS="-lstdc++"'


export LLVM_VERSION=20

export PATH=/usr/lib/llvm-$LLVM_VERSION/bin:$PATH

/opt/qt6/bin/cmake -G "$MAKEFILES_TYPE" \
      -DCMAKE_C_COMPILER=clang-$LLVM_VERSION \
      -DCMAKE_CXX_COMPILER=clang-$LLVM_VERSION \
      -DLLVM_DIR=/usr/lib/llvm-$LLVM_VERSION/cmake \
      -DClang_DIR=/usr/lib/llvm-$LLVM_VERSION/cmake \
      -DCMAKE_INSTALL_PREFIX=/opt/clazy \
      -DCMAKE_BUILD_TYPE=Release \
      -Wno-dev \
      $Options \
      .. \

