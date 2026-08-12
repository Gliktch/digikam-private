#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(pnt_SRCS PntPlugin.cpp PntRunner.cpp)

marble_add_plugin(NAME    PntPlugin
                  SOURCES ${pnt_SRCS})
