#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(graticule_SRCS GraticulePlugin.cpp)
set(graticule_UI GraticuleConfigWidget.ui)

ki18n_wrap_ui(graticule_SRCS ${graticule_UI})

marble_add_plugin(NAME GraticulePlugin
                  SOURCES ${graticule_SRCS}
)
