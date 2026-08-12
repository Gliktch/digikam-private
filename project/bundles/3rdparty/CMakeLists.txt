# Rules to build 3rd-party libraries for digiKam bundles.
#
# SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Lower minimal version of cmake required by Ubuntu 20.04.
cmake_minimum_required(VERSION 3.6)

project(digikam-bundles)

if(POLICY CMP0135)

    cmake_policy(SET CMP0135 OLD)

endif()

function(JoinListAsString VALUES GLUE OUTPUT)
    string(REPLACE ";" "${GLUE}" _TMP_STR "${VALUES}")
    set(${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Compiling in the source directory is not supported. Use for example 'mkdir build; cd build; cmake ..'.")
endif()

# Tools must be obtained to work with:
include (ExternalProject)

# allow specification of a directory with pre-downloaded
# requirements
if(NOT IS_DIRECTORY ${EXTERNALS_DOWNLOAD_DIR})
    message(FATAL_ERROR "No externals download dir set. Use -DEXTERNALS_DOWNLOAD_DIR")
endif()

if(NOT IS_DIRECTORY ${INSTALL_ROOT})
    message(FATAL_ERROR "No install dir set. Use -DINSTALL_ROOT")
endif()

set(TOP_INST_DIR ${INSTALL_ROOT})
set(EXTPREFIX "${TOP_INST_DIR}")
set(CMAKE_PREFIX_PATH "${EXTPREFIX}")

message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_CL_64:     ${CMAKE_CL_64}")

set(GLOBAL_BUILD_TYPE RelWithDebInfo)
set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DBUILD_TESTING=false)
set(PATCH_COMMAND patch)

# Function to get MD5 sums from download area for a KF5 application
# The sum is available in ${KF5_APPLICATION_MD5SUM}
function(GetKF5ApplicationMd5Sums
         name
         version)

    if(APPLE)

        # Extract MD5 sums of KF5 application directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    else() # Linux and Windows

        # Extract MD5 sums of KF5 application directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    endif()

    message(STATUS "--------------------------------")
    message(STATUS "KDE application    : ${name}")
    message(STATUS "application version: ${version}")
    message(STATUS "application MD5    : ${md5sm}")

    set(${KF5_APPLICATION_MD5SUM} ${md5sm} PARENT_SCOPE)

endfunction()

# Function to get MD5 sums from download area for a Plasma component
# The sum is available in ${KF5_PLASMA_MD5SUM}
function(GetKF5PlasmaMd5Sums
         name
         version)

    if(APPLE)

        # Extract MD5 sums of KF5 Plasma component directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/plasma/${version}/${name}-${version}.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    else() # Linux

        # Extract MD5 sums of KF5 Plasma component directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/plasma/${version}/${name}-${version}.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    endif()

    message(STATUS "--------------------------------")
    message(STATUS "KDE Plasma component: ${name}")
    message(STATUS "component version   : ${version}")
    message(STATUS "component  MD5      : ${md5sm}")

    set(${KF5_PLASMA_MD5SUM} ${md5sm} PARENT_SCOPE)

endfunction()

# this list must be dependency-ordered

add_subdirectory(ext_cmake)
add_subdirectory(ext_jpeg)
add_subdirectory(ext_jasper)
add_subdirectory(ext_png)
add_subdirectory(ext_tiff)
add_subdirectory(ext_heif)
add_subdirectory(ext_libgphoto2)
add_subdirectory(ext_libde265)
add_subdirectory(ext_libaom)
add_subdirectory(ext_libavif)
add_subdirectory(ext_libicu)
add_subdirectory(ext_libjxl)
add_subdirectory(ext_sane)
add_subdirectory(ext_exiv2)
add_subdirectory(ext_boost)
add_subdirectory(ext_opencv)
add_subdirectory(ext_openssl)
add_subdirectory(ext_lensfun)
add_subdirectory(ext_qt${ENABLE_QTVERSION})
add_subdirectory(ext_fcitx-qt)
add_subdirectory(ext_ffmpeg)
add_subdirectory(ext_imagemagick)

add_subdirectory(ext_kf${ENABLE_QTVERSION})
add_subdirectory(ext_mosaicwall)
add_subdirectory(ext_flowview)
add_subdirectory(ext_gmicqt)

if (${ENABLE_QTVERSION} MATCHES "5")

    # included in KF6 target.
    add_subdirectory(ext_plasma-wayland-protocols)
    add_subdirectory(ext_libksane)
    add_subdirectory(ext_breeze)

endif()
