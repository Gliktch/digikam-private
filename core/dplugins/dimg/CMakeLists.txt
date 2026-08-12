#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_subdirectory(jpeg)
add_subdirectory(png)
add_subdirectory(tiff)
add_subdirectory(pgf)
add_subdirectory(raw)
add_subdirectory(qimage)

if(HEIF_FOUND)
    add_subdirectory(heif)
endif()

if(Jasper_FOUND)
    add_subdirectory(jpeg2000)
endif()

if(Magick_FOUND)
    add_subdirectory(magick)
endif()
