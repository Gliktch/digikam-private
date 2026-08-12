#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(crosshairs_SRCS CrosshairsPlugin.cpp)
set(crosshairs_UI CrosshairsConfigWidget.ui)

ki18n_wrap_ui(crosshairs_SRCS ${crosshairs_UI})

qt_add_resources(crosshairs_SRCS crosshairs.qrc)

set(CrosshairsPlugin_LIBS Qt6::Svg)

marble_add_plugin(NAME    CrosshairsPlugin
                  SOURCES ${crosshairs_SRCS}
)
