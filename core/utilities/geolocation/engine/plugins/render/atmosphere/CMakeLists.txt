#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(atmosphere_SRCS AtmospherePlugin.cpp)

marble_add_plugin(NAME AtmospherePlugin
                  SOURCES ${atmosphere_SRCS}
)
