#!/bin/bash

# Copyright (c) 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Copy this script on root folder where are source code

#export VERBOSE=1

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export Options='-DBUILD_SHARED_LIBS=ON \
                -DBUILD_TESTING=OFF \
                -DJPEGXL_ENABLE_EXAMPLES=OFF \
                -DJPEGXL_ENABLE_PLUGINS=OFF \
                -DJPEGXL_ENABLE_TOOLS=OFF \
                -DJPEGXL_ENABLE_FUZZERS=OFF \
                -DJPEGXL_ENABLE_DEVTOOLS=OFF \
                -DJPEGXL_ENABLE_JPEGLI=OFF \
                -DJPEGXL_ENABLE_JPEGLI_LIBJPEG=ON \
                -DJPEGXL_ENABLE_DOXYGEN=OFF \
                -DJPEGXL_ENABLE_MANPAGES=OFF \
                -DJPEGXL_ENABLE_BENCHMARK=OFF \
                -DJPEGXL_BUNDLE_LIBPNG=OFF \
                -DJPEGXL_ENABLE_JNI=OFF \
                -DJPEGXL_ENABLE_SJPEG=OFF \
                -DJPEGXL_ENABLE_OPENEXR=OFF \
                -DJPEGXL_ENABLE_SKCMS=OFF \
                -DJPEGXL_BUNDLE_SKCMS=OFF \
                -DJPEGXL_ENABLE_VIEWERS=OFF \
                -DJPEGXL_ENABLE_TCMALLOC=OFF \
                -DJPEGXL_ENABLE_COVERAGE=OFF \
                -DJPEGXL_ENABLE_PROFILER=OFF \
                -DJPEGXL_ENABLE_SIZELESS_VECTORS=OFF \
                -DJPEGXL_ENABLE_TRANSCODE_JPEG=ON \
                -DJPEGXL_ENABLE_BOXES=ON \
                -DJPEGXL_STATIC=OFF \
                -DJPEGXL_WARNINGS_AS_ERRORS=OFF \
                -DJPEGXL_FORCE_NEON=OFF \
                -DJPEGXL_FORCE_SYSTEM_BROTLI=OFF \
                -DJPEGXL_FORCE_SYSTEM_GTEST=OFF \
                -DJPEGXL_FORCE_SYSTEM_LCMS2=OFF \
                -DJPEGXL_FORCE_SYSTEM_HWY=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $Options \
      ..

