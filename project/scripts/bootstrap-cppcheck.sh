#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code
# https://github.com/danmar/cppcheck

#export CC=/usr/bin/clang
#export CXX=/usr/bin/clang++

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export Options='-DANALYZE_MEMORY=OFF \
                -DANALYZE_ADDRESS=OFF \
                -DANALYZE_THREAD=OFF \
                -DANALYZE_UNDEFINED=OFF \
                -DANALYZE_DATAFLOW=OFF \
                -DWARNINGS_ARE_ERRORS=OFF \
                -DENABLE_CHECK_INTERNAL=OFF \
                -DBUILD_CORE_DLL=OFF \
                -DBUILD_TESTS=OFF \
                -DREGISTER_TESTS=OFF \
                -DDISABLE_DMAKE=OFF \
                -DBUILD_MANPAGE=OFF \
                -DBUILD_CLI=ON \
                -DBUILD_GUI=OFF \
                -DWITH_QCHART=OFF \
                -DUSE_QT6=OFF \
                -DREGISTER_GUI_TESTS=OFF \
                -DBUILD_ONLINE_HELP=OFF \
                -DHAVE_RULES=OFF \
                -DUSE_BUNDLED_TINYXML2=ON \
                -DCPPCHK_GLIBCXX_DEBUG=OFF \
                -DDISALLOW_THREAD_EXECUTOR=OFF \
                -DUSE_BOOST=OFF \
                -DUSE_BOOST_INT128=OFF \
                -DUSE_LIBCXX=OFF \
                -DDISABLE_CRTDBG_MAP_ALLOC=OFF \
                -DNO_UNIX_SIGNAL_HANDLING=OFF \
                -DNO_UNIX_BACKTRACE_SUPPORT=OFF \
                -DNO_WINDOWS_SEH=OFF \
                -DFILESDIR=OFF \
                '

/opt/qt6/bin/cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/opt/cppcheck \
      -Wno-dev \
      -DCMAKE_BUILD_TYPE=Release \
      $Options \
      ..

