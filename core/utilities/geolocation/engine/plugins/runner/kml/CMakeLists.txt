#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(kml_SRCS KmlParser.cpp KmlPlugin.cpp KmlRunner.cpp)

marble_add_plugin(NAME    KmlPlugin
                  SOURCES ${kml_SRCS})
