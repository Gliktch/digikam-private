#!/bin/bash

# Copyright (c) 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Copy this script on root folder where are source code

#export VERBOSE=1

if [ ! -d "build.tmp" ]; then
    mkdir build.tmp
fi

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

cmake -H./ \
      -B./build.tmp \
      -G "$MAKEFILES_TYPE" \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      -DJAS_ENABLE_SHARED=ON \
      -DJAS_ENABLE_HIDDEN=ON \
      -DJAS_ENABLE_32BIT=OFF \
      -DJAS_ENABLE_LIBJPEG=ON \
      -DJAS_ENABLE_LIBHEIF=OFF \
      -DJAS_ENABLE_OPENGL=ON \
      -DJAS_ENABLE_DOC=OFF \
      -DJAS_ENABLE_LATEX=OFF \
      -DJAS_ENABLE_PROGRAMS=OFF \
      -DJAS_ENABLE_MULTITHREADING_SUPPORT=ON \
      -DJAS_PREFER_PTHREAD=ON \
      -DJAS_PREFER_PTHREAD_TSS=OFF \
      -DJAS_STRICT=OFF \
      -DALLOW_IN_SOURCE_BUILD=ON
