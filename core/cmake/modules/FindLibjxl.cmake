# - Try to find JXL library (https://github.com/libjxl/libjxl)
# Once done this will define
#
# JXL_FOUND            True if JXL library was found.
# JXL_INCLUDE_DIRS     Where to find JXL library headers
# JXL_LIBRARIES        List of libraries to link against when using JXL library
# JXL_DEFINITIONS      Compiler switches required for using JXL library
# JXL_VERSION          Version of JXL library (e.g., 1.12.0)
#
# SPDX-FileCopyrightText: 2019-2024 Caulier Gilles <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(JXL_LIBRARIES AND JXL_INCLUDE_DIRS AND JXL_VERSION AND JXL_DEFINITIONS)

    # In cache already

    Set(JXL_FOUND TRUE)

else()

    # Use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls

    if(NOT WIN32)

        if(NOT PKG_CONFIG_FOUND)

            find_package(PkgConfig)

        endif()

        if(PKG_CONFIG_FOUND)

           PKG_CHECK_MODULES(PC_JXL jxl QUIET)

        endif()

    endif()

    set(JXL_DEFINITIONS ${PC_JXL_CFLAGS_OTHER})

    include(FindPackageHandleStandardArgs)

    find_path(JXL_INCLUDES_DIR jxl/version.h
              HINTS            ${PC_JXL_INCLUDE_DIRS}
    )

    find_library(JXL_LIBRARIES jxl
                 HINTS         ${PC_JXL_LIBRARY_DIRS}
    )

    find_library(JXL_THREADS_LIBRARIES jxl_threads
                 HINTS                 ${PC_JXL_LIBRARY_DIRS}
    )

    if(JXL_INCLUDES_DIR)

        file(STRINGS "${JXL_INCLUDES_DIR}/jxl/version.h" TMP REGEX "^#define JPEGXL_MAJOR_VERSION.*$")
        string(REGEX MATCHALL "[0-9.]+" JXL_MAJOR_VERSION ${TMP})

        file(STRINGS "${JXL_INCLUDES_DIR}/jxl/version.h" TMP REGEX "^#define JPEGXL_MINOR_VERSION.*$")
        string(REGEX MATCHALL "[0-9.]+" JXL_MINOR_VERSION ${TMP})

        file(STRINGS "${JXL_INCLUDES_DIR}/jxl/version.h" TMP REGEX "^#define JPEGXL_PATCH_VERSION.*$")
        string(REGEX MATCHALL "[0-9.]+" JXL_PATCH_VERSION ${TMP})

        set(JXL_VERSION "${JXL_MAJOR_VERSION}.${JXL_MINOR_VERSION}.${JXL_PATCH_VERSION}")

    endif()

    find_package_handle_standard_args(Libjxl
                                      REQUIRED_VARS
                                        JXL_INCLUDES_DIR
                                        JXL_LIBRARIES
                                        JXL_THREADS_LIBRARIES
    )

    if(Libjxl_FOUND)

        set(JXL_FOUND        ${Libjxl_FOUND})
        set(JXL_INCLUDE_DIRS ${JXL_INCLUDES_DIR})

        if(NOT TARGET Libjxl::Libjxl)

            add_library(Libjxl::Libjxl UNKNOWN IMPORTED)

            set_target_properties(Libjxl::Libjxl PROPERTIES
                                  INTERFACE_INCLUDE_DIRECTORIES "${JXL_INCLUDE_DIRS}")

            set_property(TARGET Libjxl::Libjxl APPEND PROPERTY
                         IMPORTED_LOCATION "${JXL_LIBRARIES}")

        endif()

        set(JXL_LIBRARIES ${JXL_LIBRARIES} ${JXL_THREADS_LIBRARIES})

        mark_as_advanced(JXL_LIBRARIES JXL_INCLUDE_DIRS JXL_VERSION JXL_DEFINITIONS)

    endif()

    message(STATUS "JXL_FOUND        = ${JXL_FOUND}")
    message(STATUS "JXL_INCLUDE_DIRS = ${JXL_INCLUDE_DIRS}")
    message(STATUS "JXL_LIBRARIES    = ${JXL_LIBRARIES}")
    message(STATUS "JXL_DEFINITIONS  = ${JXL_DEFINITIONS}")
    message(STATUS "JXL_VERSION      = ${JXL_VERSION}")

endif()
