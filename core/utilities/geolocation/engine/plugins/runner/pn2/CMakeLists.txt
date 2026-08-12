#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(pn2_SRCS Pn2Plugin.cpp Pn2Runner.cpp)

marble_add_plugin(NAME    Pn2Plugin
                  SOURCES ${pn2_SRCS})
