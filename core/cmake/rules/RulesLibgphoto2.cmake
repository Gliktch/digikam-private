#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(Gphoto2)

if(Gphoto2_FOUND)

    if("${GPHOTO2_VERSION_STRING}" VERSION_GREATER "2.4.0")

        set(VERSION_GPHOTO2 true)

    else()

        set(VERSION_GPHOTO2 false)

    endif()

    if("${GPHOTO2_VERSION_STRING}" VERSION_GREATER "2.5.0")

        set(VERSION_GPHOTO25 true)
        message(STATUS "libgphoto2 API version >= 2.5")

    else()

        set(VERSION_GPHOTO25 false)
        message(STATUS "libgphoto2 API version < 2.5")

    endif()

    if(VERSION_GPHOTO25)

        set(HAVE_GPHOTO25 1)

    else()

        set(HAVE_GPHOTO25 0)

    endif()

endif()
