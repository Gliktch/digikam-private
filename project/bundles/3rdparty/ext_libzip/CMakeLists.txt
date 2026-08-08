# Script to build libzip for the digiKam Private AppImage.
#
# SPDX-FileCopyrightText: 2026 by digiKam Private contributors
# SPDX-License-Identifier: BSD-3-Clause

set(PREFIX_ext_libzip "${EXTPREFIX}")

ExternalProject_Add(ext_libzip
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/nih-at/libzip/releases/download/v1.11.4/libzip-1.11.4.tar.xz
    URL_HASH SHA256=8a247f57d1e3e6f6d11413b12a6f28a9d388de110adc0ec608d893180ed7097b

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libzip}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               -DBUILD_SHARED_LIBS=ON
               -DBUILD_TOOLS=OFF
               -DBUILD_REGRESS=OFF
               -DBUILD_EXAMPLES=OFF
               -DBUILD_DOC=OFF
               -DBUILD_OSSFUZZ=OFF
               -DENABLE_BZIP2=OFF
               -DENABLE_LZMA=OFF
               -DENABLE_ZSTD=OFF
               -DENABLE_COMMONCRYPTO=OFF
               -DENABLE_GNUTLS=OFF
               -DENABLE_MBEDTLS=OFF
               -DENABLE_OPENSSL=OFF
               -DENABLE_WINDOWS_CRYPTO=OFF
               ${GLOBAL_PROFILE}

    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config ${GLOBAL_BUILD_TYPE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTPREFIX}/share/licenses/digikam-private
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    <SOURCE_DIR>/LICENSE
                    ${EXTPREFIX}/share/licenses/digikam-private/libzip-LICENSE

    UPDATE_COMMAND ""
    ALWAYS 0
)
