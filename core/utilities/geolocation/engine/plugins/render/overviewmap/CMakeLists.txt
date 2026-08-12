#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(overviewmap_SRCS OverviewMap.cpp)
set(overviewmap_UI OverviewMapConfigWidget.ui)

ki18n_wrap_ui(overviewmap_SRCS ${overviewmap_UI})

marble_add_plugin(NAME    OverviewMap
                  SOURCES ${overviewmap_SRCS})
