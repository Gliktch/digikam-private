#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(mapscale_SRCS MapScaleFloatItem.cpp)
set(mapscale_UI MapScaleConfigWidget.ui)

ki18n_wrap_ui(mapscale_SRCS ${mapscale_UI})

marble_add_plugin(NAME    MapScaleFloatItem
                  SOURCES ${mapscale_SRCS})
