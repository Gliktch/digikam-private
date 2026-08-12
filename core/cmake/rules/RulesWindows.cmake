#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(MSVC)

    # For LibRaw

    set(WSOCK32_LIBRARY  wsock32)
    set(WS2_32_LIBRARY   ws2_32)
    set(BCRYPT_LIBRARY   bcrypt)

    # For MediaPlayer

    set(NETAPI32_LIBRARY netapi32)
    set(USEENV_LIBRARY   userenv)
    set(PSAPI_LIBRARY    psapi)

    # For the color Management

    set(MSCMS_LIBRARY     mscms)

    # Increase the stack size to prevent crashes with GMic-qt.

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:16777216")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /STACK:16777216")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /STACK:16777216")

    # --- Disable indeep warnings from Visual Studio C++

    # To disable warnings about no suitable definition provided for explicit template instantiation request.
    add_compile_options(/wd4661)

    # To disable warnings about deprecated POSIX methods().
    add_compile_options(/wd4996)

    # To disable warnings about qualifier applied to reference type ignored.
    add_compile_options(/wd4181)

    # To disable warnings about 'storage-class-keyword' no longer supported as storage class.
    add_compile_options(/wd5033)

    # To disable warnings about size_t to uint conversion data lost.
    add_compile_options(/wd4267)

    # To disable warnings about truncation from double to float.
    add_compile_options(/wd4305)

    # To disable warnings about enum has named enumerators with values that cannot be represented in the given bit field width.
    add_compile_options(/wd5249)

    # --- Disable linking warnings

    # To disable warnings about linking object specified multiple times.
    add_link_options(/ignore:4197)

    # Remove this warning flag force by VCPKG which disable /wd5249

    string(REPLACE "/w35249" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/w35249" "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")

endif()
