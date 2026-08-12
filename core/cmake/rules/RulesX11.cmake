#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# NOTE: X11 API are used to manage Monitor Profiles with LCMS

find_package(X11)

if(X11_FOUND)

    if(NOT Qt6_FOUND)

        find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE
                                           COMPONENTS
                                           X11Extras
        )

    endif()

    set(HAVE_X11 TRUE)

    if (X11_Xv_FOUND)

        set(HAVE_XV TRUE)

    else()

        set(HAVE_XV FALSE)

    endif()

else()

    set(HAVE_X11 FALSE)

endif()

# For MediaPlayer linking

MACRO_BOOL_TO_01(HAVE_X11 HAVE_LIBX11)
MACRO_BOOL_TO_01(HAVE_XV  HAVE_LIBXV)
