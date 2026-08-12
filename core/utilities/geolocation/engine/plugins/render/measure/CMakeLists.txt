#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(measuretool_SRCS MeasureToolPlugin.cpp MeasureConfigDialog.cpp)
set(measuretool_UI MeasureConfigDialog.ui)

ki18n_wrap_ui(measuretool_SRCS ${measuretool_UI})

qt_add_resources(measuretool_SRCS measuretool.qrc)

marble_add_plugin(NAME    MeasureTool
                  SOURCES ${measuretool_SRCS})
