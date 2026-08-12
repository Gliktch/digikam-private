#!/bin/bash

# SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code

./configure --prefix=/usr                \
            --extra-libs=-lstdc++        \
            --disable-doc                \
            --disable-debug              \
            --disable-programs           \
            --disable-outdevs            \
            --enable-shared              \
            --enable-gpl                 \
            --enable-version3            \
            --enable-nonfree             \
            --enable-runtime-cpudetect   \
            --enable-libopencore-amrnb   \
            --enable-libopencore-amrwb   \
            --enable-libfdk-aac          \
            --enable-libmp3lame          \
            --enable-libspeex            \
            --enable-libtheora           \
            --enable-libvorbis           \
            --enable-libx264             \
            --enable-libx265             \
            --enable-libaom              \
            --enable-libxvid             \
            --enable-libvpx              \
            --enable-libopus             \
            --enable-librtmp             \
            --enable-libfreetype         \
            --enable-libass              \
            --enable-opengl              \
