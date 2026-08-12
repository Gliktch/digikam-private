#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_subdirectory(antivignetting)
add_subdirectory(blur)
add_subdirectory(hotpixels)
add_subdirectory(redeye)
add_subdirectory(sharpen)
add_subdirectory(restoration)
add_subdirectory(localcontrast)
add_subdirectory(noisereduction)
add_subdirectory(lensdistortion)
add_subdirectory(healingclone)

if(LensFun_FOUND)
    add_subdirectory(lensautofix)
endif()

# Not yet finalized
add_subdirectory(backgroundblur)

