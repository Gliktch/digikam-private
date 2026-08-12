# Some useful cmake macros for general purposes
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# -------------------------------------------------------------------------

set(PRINT_COMPILE_LENGTH "40")

macro(FILL_WITH_DOTS VAR)

    string(LENGTH ${${VAR}} NAME_LENGTH)

    math(EXPR DOT_LENGTH "${PRINT_COMPILE_LENGTH} - ${NAME_LENGTH}")

    if(${DOT_LENGTH} LESS 0)

        set(DOT_LENGTH 0)

    endif()

    foreach(COUNT RANGE ${DOT_LENGTH})

        set(${VAR} "${${VAR}}.")

    endforeach()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_LIBRARY_STATUS NAME WEBSITE VERSIONHINT)

    set(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES")

    else()

        message(STATUS " ${LIB_MESSAGE} NO")
        message(SEND_ERROR " ${NAME} is needed. You need to install the ${NAME} ${VERSIONHINT} development package.")
        message(STATUS " ${NAME} website is at ${WEBSITE}")
        message(STATUS "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_OPTIONAL_LIBRARY_STATUS NAME WEBSITE VERSIONHINT FEATUREMISSING)

    set(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES (optional)")

    else()

        message(STATUS " ${LIB_MESSAGE} NO  (optional)")
        message(STATUS " ${FEATUREMISSING}")
        message(STATUS " Please install the ${NAME} ${VERSIONHINT} development package.")
        if(${WEBSITE})
            message(STATUS " ${NAME} website is at ${WEBSITE}")
        endif()
        message(STATUS "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_QTMODULE_STATUS NAME)

    set(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES")

    else()

        message(STATUS " ${LIB_MESSAGE} NO")
        message(STATUS "")
        message(SEND_ERROR " ${NAME} module is needed. You need to install a package containing the ${NAME} module.")
        message(STATUS "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_EXECUTABLE_STATUS NAME TECHNICAL_NAME PATH_VARIABLE)

    set(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES")

    else()

        message(STATUS " ${LIB_MESSAGE} NO")
        message(STATUS "")
        message(STATUS " ${NAME} is needed. You need to install the package containing the \"${TECHNICAL_NAME}\" executable.")
        message(STATUS " If you have this executable installed, please specify the folder containing it by ${PATH_VARIABLE}")
        message(SEND_ERROR "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_COMPONENT_COMPILE_STATUS NAME)

    set(COMPILE_MESSAGE "${NAME} will be compiled")
    FILL_WITH_DOTS(COMPILE_MESSAGE)

    IF(${ARGN})

        message(STATUS " ${COMPILE_MESSAGE} YES (optional)")

    else()

        message(STATUS " ${COMPILE_MESSAGE} NO  (optional)")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_OPTIONAL_QTMODULE_STATUS NAME)

    set(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES (optional)")

    else()

        message(STATUS " ${LIB_MESSAGE} NO  (optional)")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(HEADER_DIRECTORIES root_path return_list)

    file(GLOB_RECURSE new_list ${root_path}/*.h)
    set(dir_list "")

    foreach(file_path ${new_list})

        get_filename_component(dir_path ${file_path} PATH)
        set(dir_list ${dir_list} ${dir_path})

    endforeach()

    list(REMOVE_DUPLICATES dir_list)
    set(${return_list} ${dir_list})

endmacro()

# -------------------------------------------------------------------------

macro(APPLY_COMMON_POLICIES)

    if(POLICY CMP0063)
        # C++ symbols visibility policy introduced in CMake version 3.3
        # Details: https://cmake.org/cmake/help/git-stage/policy/CMP0063.html
        cmake_policy(SET CMP0063 NEW)
    endif()

    if(POLICY CMP0068)
        # MacOS RPATH settings policy introduced in CMake version 3.9
        # Details: https://cmake.org/cmake/help/git-stage/policy/CMP0068.html
        cmake_policy(SET CMP0068 NEW)
    endif()

    if(POLICY CMP0071)
        # Automoc/autouic files handling introduced in CMake version 3.10
        # Details: https://cmake.org/cmake/help/git-stage/policy/CMP0071.html
        cmake_policy(SET CMP0071 NEW)
    endif()

    if(POLICY CMP0092)
        # MSVC warnings flag rules introduced in CMake version 3.16
        # Details: https://cmake.org/cmake/help/git-stage/policy/CMP0092.html
        cmake_policy(SET CMP0092 NEW)
    endif()

    if(POLICY CMP0177)
        # Destination paths are normalized introduced in CMake version 3.31
        # Details: https://cmake.org/cmake/help/git-stage/policy/CMP0177.html
        cmake_policy(SET CMP0177 OLD)
    endif()

endmacro()

# -------------------------------------------------------------------------

macro(MACOS_DEBUG_POLICIES)

    # Cmake do not support yet the dSYM scheme.
    # See details from the CMake report: https://gitlab.kitware.com/cmake/cmake/-/issues/20256

    if(APPLE)

        if((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))

            message(STATUS "MacOS optimized build, symbol generation turned-OFF" )

            # on optimized builds, do NOT turn-on symbol generation.

        else()

            message(STATUS "MacOS non-optimized build, symbol generation turned-ON")

            # Incredibly, for both clang and g++, while a single compile-and-link
            # invocation will create an executable.dSYM/ dir with debug info,
            # with separate compilation the final link does NOT create the
            # dSYM dir.
            # The "dsymutil" program will create the dSYM dir for us.
            # Strangely it takes in the executable and not the object
            # files even though it's the latter that contain the debug info.
            # Thus it will only work if the object files are still sitting around.

            find_program(DSYMUTIL_PROGRAM dsymutil)

            if (DSYMUTIL_PROGRAM)

                set(CMAKE_C_LINK_EXECUTABLE
                    "${CMAKE_C_LINK_EXECUTABLE}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_C_CREATE_SHARED_LIBRARY
                    "${CMAKE_C_CREATE_SHARED_LIBRARY}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_CXX_LINK_EXECUTABLE
                    "${CMAKE_CXX_LINK_EXECUTABLE}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_CXX_CREATE_SHARED_LIBRARY
                    "${CMAKE_CXX_CREATE_SHARED_LIBRARY}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_CXX_CREATE_SHARED_MODULE
                    "${CMAKE_CXX_CREATE_SHARED_MODULE}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

            endif()

        endif()

    endif()

endmacro()

