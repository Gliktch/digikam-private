#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(ImageMagick COMPONENTS Magick++
                                    MagickCore
                                    MagickWand)

if(ImageMagick_FOUND)

    set(Magick_FOUND TRUE)
    set(Magick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS})
    set(Magick_LIBRARIES ${ImageMagick_LIBRARIES})
    include_directories(${Magick_INCLUDE_DIRS})

    if(ImageMagick_Magick++_FOUND)

        set(Magick++_DEFINITIONS ${ImageMagick_Magick++_DEFINITIONS})
        add_definitions(${Magick++_DEFINITIONS})

    endif()

endif()
