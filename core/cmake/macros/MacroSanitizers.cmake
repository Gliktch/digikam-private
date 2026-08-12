# An useful macro to manage sanitizer rules
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

macro(ENABLE_COMPILER_SANITIZERS)

    if(ENABLE_SANITIZERS)

        # NOTE: just one sanitizer can be used at configuration time, else compilation fail.

        include(FindASan)
#        include(FindUBSan)
#        include(FindMSan)
#        include(FindTSan)

        if (HAVE_ADDRESS_SANITIZER)

            message(STATUS "Found AddressSanitizer library: ${CMAKE_CXX_FLAGS_ASAN}")
            set(ASAN_FOUND TRUE CACHE INTERNAL "")
            set(ECM_ENABLE_SANITIZERS ${ECM_ENABLE_SANITIZERS} "address")

        else()

            message(STATUS "Could not find AddressSanitizer library. Please install libasan...")

        endif()

        ###

        if (HAVE_UNDEFINED_BEHAVIOR_SANITIZER)

            message(STATUS "Found UndefinedBehaviorSanitizer library: ${CMAKE_CXX_FLAGS_UBSAN}")
            set(UBSAN_FOUND TRUE CACHE INTERNAL "")
            set(ECM_ENABLE_SANITIZERS ${ECM_ENABLE_SANITIZERS} "undefined")

        else()

            message(STATUS "Could not find UndefinedBehaviorSanitizer library. Please install libubsan...")

        endif()

        ###

        if (HAVE_MEMORY_SANITIZER)

            message(STATUS "Found MemorySanitizer library: ${CMAKE_CXX_FLAGS_MSAN}")
            set(MSAN_FOUND TRUE CACHE INTERNAL "")
            set(ECM_ENABLE_SANITIZERS ${ECM_ENABLE_SANITIZERS} "memory")

        else()

            message(STATUS "Could not find MemorySanitizer library. Please install libmsan...")

        endif()

        ###

        if (HAVE_THREAD_SANITIZER)

            message(STATUS "Found ThreadSanitizer library: ${CMAKE_CXX_FLAGS_TSAN}")
            set(TSAN_FOUND TRUE CACHE INTERNAL "")
            set(ECM_ENABLE_SANITIZERS ${ECM_ENABLE_SANITIZERS} "thread")

        else()

            message(STATUS "Could not find ThreadSanitizer library. Please install libtsan...")

        endif()

        ###

        message(STATUS "Building with sanitizers: ${ECM_ENABLE_SANITIZERS}")

        if ("${ECM_ENABLE_SANITIZERS}" STREQUAL "")

            message(FATAL_ERROR "Could not find Sanitizer libraries")

        endif()

        include(ECMEnableSanitizers)

    endif()

endmacro()

