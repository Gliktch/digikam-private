#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(LensFun)

if(LENSFUN_VERSION)

    message(STATUS "liblensfun: Found version ${LENSFUN_VERSION} (required: ${LENSFUN_MIN_VERSION})")

    if(${LENSFUN_VERSION} VERSION_LESS ${LENSFUN_MIN_VERSION})

        set(LensFun_FOUND FALSE)

    endif()

else()

    message(STATUS "liblensfun: Version information not found, your version is probably too old.")
    set(LensFun_FOUND FALSE)

endif()
