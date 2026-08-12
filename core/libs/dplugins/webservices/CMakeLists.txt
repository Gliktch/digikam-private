#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# OAuth2 library ---------------------------------------------------

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050E00
    -DQT_NO_KEYWORDS
    -DQT_NO_FOREACH
)

# Remove unwanted warnings from O2 library
REMOVE_GCC_COMPILER_WARNINGS("-pedantic")
DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-extra-semi")

# It also does not export symbols, so export all by default
unset(CMAKE_CXX_VISIBILITY_PRESET)

set(libwso2_SRCS
    o2/src/o2.cpp
    o2/src/o2reply.cpp
    o2/src/o2replyserver.cpp
    o2/src/o0jsonresponse.cpp
    o2/src/o2requestor.cpp
    o2/src/o2simplecrypt.cpp
    o2/src/o0settingsstore.cpp
    o2/src/o0baseauth.cpp
    o2/src/o0abstractstore.h
    o2/src/o0globals.h
    # Enable when needed
    o2/src/o1.cpp
    o2/src/o1requestor.cpp
    o2/src/o1timedreply.cpp
    o2/src/o1smugmug.cpp
    o2/src/o1twitter.h
    o2/src/oxtwitter.cpp
    #o2/src/o1dropbox.h
    #o2/src/o1flickr.h
    #o2/src/o2gft.cpp
    o2/src/o2pollserver.cpp
    o2/src/o2facebook.cpp
    #o2/src/o2skydrive.cpp
    #o2/src/o2hubic.cpp
    #o2/src/o2uber.cpp
    #o2/src/o2msgraph.cpp
    #o2/src/o2vimeo.cpp
)

# Used by digikamcore
add_library(core_libwso2_obj OBJECT ${libwso2_SRCS})

target_compile_definitions(core_libwso2_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
