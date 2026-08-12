#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_subdirectory(antivignetting)
add_subdirectory(blur)
add_subdirectory(localcontrast)
add_subdirectory(noisereduction)
add_subdirectory(redeyecorrection)
add_subdirectory(restoration)
add_subdirectory(sharpen)
add_subdirectory(hotpixels)

if(LensFun_FOUND)
    add_subdirectory(lensautofix)
endif()
