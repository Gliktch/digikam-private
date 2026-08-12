# - Try to find HEIF library (https://github.com/strukturag/libheif)
# Once done this will define
#
# HEIF_FOUND            True if HEIF library was found.
# HEIF_INCLUDE_DIRS     Where to find HEIF library headers
# HEIF_LIBRARIES        List of libraries to link against when using HEIF library
# HEIF_DEFINITIONS      Compiler switches required for using HEIF library
# HEIF_VERSION          Version of HEIF library (e.g., 1.12.0)
#
# SPDX-FileCopyrightText: 2019-2024 Caulier Gilles <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(HEIF_LIBRARIES AND HEIF_INCLUDE_DIRS AND HEIF_VERSION AND HEIF_DEFINITIONS)

    # In cache already

    Set(HEIF_FOUND TRUE)

else()

    # Use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls

    if(NOT WIN32)

        if(NOT PKG_CONFIG_FOUND)

            find_package(PkgConfig)

        endif()

        if(PKG_CONFIG_FOUND)

           PKG_CHECK_MODULES(PC_HEIF libheif QUIET)

        endif()

    endif()

    set(HEIF_DEFINITIONS ${PC_HEIF_CFLAGS_OTHER})

    include(FindPackageHandleStandardArgs)

    find_path(HEIF_INCLUDES_DIR libheif/heif_version.h
              HINTS             ${PC_HEIF_INCLUDE_DIRS}
    )

    find_library(HEIF_LIBRARIES heif
                 HINTS          ${PC_HEIF_LIBRARY_DIRS}
    )

    if(HEIF_INCLUDES_DIR)

        file(STRINGS "${HEIF_INCLUDES_DIR}/libheif/heif_version.h" TMP REGEX "^#define LIBHEIF_VERSION[ \t].*$")
        string(REGEX MATCHALL "[0-9.]+" HEIF_VERSION "${TMP}")

    endif()

    find_package_handle_standard_args(Libheif
                                      REQUIRED_VARS
                                        HEIF_INCLUDES_DIR
                                        HEIF_LIBRARIES
    )

    if(Libheif_FOUND)

        set(HEIF_FOUND        ${Libheif_FOUND})
        set(HEIF_INCLUDE_DIRS ${HEIF_INCLUDES_DIR})

        if(NOT TARGET Libheif::Libheif)

            add_library(Libheif::Libheif UNKNOWN IMPORTED)

            set_target_properties(Libheif::Libheif PROPERTIES
                                  INTERFACE_INCLUDE_DIRECTORIES "${HEIF_INCLUDE_DIRS}")

            set_property(TARGET Libheif::Libheif APPEND PROPERTY
                         IMPORTED_LOCATION "${HEIF_LIBRARIES}")

        endif()

    endif()

    mark_as_advanced(HEIF_INCLUDE_DIRS HEIF_LIBRARIES HEIF_VERSION HEIF_DEFINITIONS)

    message(STATUS "HEIF_FOUND        = ${HEIF_FOUND}")
    message(STATUS "HEIF_INCLUDE_DIRS = ${HEIF_INCLUDE_DIRS}")
    message(STATUS "HEIF_LIBRARIES    = ${HEIF_LIBRARIES}")
    message(STATUS "HEIF_DEFINITIONS  = ${HEIF_DEFINITIONS}")
    message(STATUS "HEIF_VERSION      = ${HEIF_VERSION}")

endif()
