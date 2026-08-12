
# This module tests if address sanitizer is supported by the compiler,
# and creates a ASan build type (i.e. set CMAKE_BUILD_TYPE=ASan to use
# it). This sets the following variables:
#
# CMAKE_C_FLAGS_ASAN     - Flags to use for C with asan
# CMAKE_CXX_FLAGS_ASAN   - Flags to use for C++ with asan
# HAVE_ADDRESS_SANITIZER - True or false if the ASan build type is available
#
# SPDX-FileCopyrightText: 2013      by Matthew Arsenault
#
# SPDX-License-Identifier: MIT
#

include(CheckCXXCompilerFlag)

# Set -Werror to catch "argument unused during compilation" warnings
set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=address") # Also needs to be a link flag for test to pass
check_cxx_compiler_flag("-fsanitize=address" HAVE_FLAG_SANITIZE_ADDRESS)
unset(CMAKE_REQUIRED_FLAGS)

if(HAVE_FLAG_SANITIZE_ADDRESS)
  # Clang 3.2+ use this version
  set(ADDRESS_SANITIZER_FLAG "-fsanitize=address")
endif()

if(NOT ADDRESS_SANITIZER_FLAG)
  return()
else(NOT ADDRESS_SANITIZER_FLAG)
  set(HAVE_ADDRESS_SANITIZER FALSE)
endif()

set(HAVE_ADDRESS_SANITIZER TRUE)

set(CMAKE_C_FLAGS_ASAN "-O0 -g ${ADDRESS_SANITIZER_FLAG} -fno-omit-frame-pointer -fno-optimize-sibling-calls"
    CACHE STRING "Flags used by the C compiler during ASan builds."
    FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-O0 -g ${ADDRESS_SANITIZER_FLAG} -fno-omit-frame-pointer -fno-optimize-sibling-calls"
    CACHE STRING "Flags used by the C++ compiler during ASan builds."
    FORCE)
set(CMAKE_EXE_LINKER_FLAGS_ASAN "${ADDRESS_SANITIZER_FLAG}"
    CACHE STRING "Flags used for linking binaries during ASan builds."
    FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_ASAN "${ADDRESS_SANITIZER_FLAG}"
    CACHE STRING "Flags used by the shared libraries linker during ASan builds."
    FORCE)

mark_as_advanced(CMAKE_C_FLAGS_ASAN
                 CMAKE_CXX_FLAGS_ASAN
                 CMAKE_EXE_LINKER_FLAGS_ASAN
                 CMAKE_SHARED_LINKER_FLAGS_ASAN)


check_cxx_compiler_flag("-Og" HAVE_OPTIMIZE_DEBUG)
if(HAVE_OPTIMIZE_DEBUG)
  set(CMAKE_C_FLAGS_ASANOPT "-Og -g ${ADDRESS_SANITIZER_FLAG} -fno-omit-frame-pointer -fno-optimize-sibling-calls"
          CACHE STRING "Flags used by the C compiler during ASan Optimized builds."
          FORCE)
  set(CMAKE_CXX_FLAGS_ASANOPT "-Og -g ${ADDRESS_SANITIZER_FLAG} -fno-omit-frame-pointer -fno-optimize-sibling-calls"
          CACHE STRING "Flags used by the C++ compiler during ASan Optimized builds."
          FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_ASANOPT "${ADDRESS_SANITIZER_FLAG}"
          CACHE STRING "Flags used for linking binaries during ASan Optimized builds."
          FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_ASANOPT "${ADDRESS_SANITIZER_FLAG}"
          CACHE STRING "Flags used by the shared libraries linker during ASan Optimized builds."
          FORCE)

  mark_as_advanced(CMAKE_C_FLAGS_ASANOPT
          CMAKE_CXX_FLAGS_ASANOPT
          CMAKE_EXE_LINKER_FLAGS_ASANOPT
          CMAKE_SHARED_LINKER_FLAGS_ASANOPT)
endif()
