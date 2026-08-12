#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(gpx_SRCS
    GpxParser.cpp
    GpxPlugin.cpp
    GpxRunner.cpp
    handlers/GPXElementDictionary.cpp
    handlers/GPXgpxTagHandler.cpp
    handlers/GPXnameTagHandler.cpp
    handlers/GPXdescTagHandler.cpp
    handlers/GPXtypeTagHandler.cpp
    handlers/GPXtrkTagHandler.cpp
    handlers/GPXtrkptTagHandler.cpp
    handlers/GPXtrksegTagHandler.cpp
    handlers/GPXwptTagHandler.cpp
    handlers/GPXtimeTagHandler.cpp
    handlers/GPXeleTagHandler.cpp
    handlers/GPXextensionsTagHandler.cpp
    handlers/GPXTrackPointExtensionTagHandler.cpp
    handlers/GPXhrTagHandler.cpp
    handlers/GPXrteTagHandler.cpp
    handlers/GPXrteptTagHandler.cpp
    handlers/GPXcmtTagHandler.cpp
    handlers/GPXlinkTagHandler.cpp
    handlers/GPXurlTagHandler.cpp
    handlers/GPXurlnameTagHandler.cpp
)

marble_add_plugin(NAME    GpxPlugin
                  SOURCES ${gpx_SRCS})
