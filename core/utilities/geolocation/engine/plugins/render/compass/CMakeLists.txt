#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(compass_SRCS CompassFloatItem.cpp)
set(compass_UI CompassConfigWidget.ui)

ki18n_wrap_ui(compass_SRCS ${compass_UI})

qt_add_resources(compass_SRCS compass.qrc)

marble_add_plugin(NAME    CompassFloatItem
                  SOURCES ${compass_SRCS})
