# Install the pinned upstream static gocryptfs tools for digiKam Private.
#
# SPDX-FileCopyrightText: 2026 by digiKam Private contributors
# SPDX-License-Identifier: BSD-3-Clause

if(UNIX AND NOT APPLE)

    ExternalProject_Add(ext_gocryptfs
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/rfjakob/gocryptfs/releases/download/v2.6.1/gocryptfs_v2.6.1_linux-static_amd64.tar.gz
        URL_HASH SHA256=49b8c0eb0f6373b6ac99c394a52909d8478e74c08d0961527c1162967cc28c44

        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTPREFIX}/bin
                COMMAND ${CMAKE_COMMAND} -E copy_if_different <SOURCE_DIR>/gocryptfs ${EXTPREFIX}/bin/gocryptfs
                COMMAND ${CMAKE_COMMAND} -E copy_if_different <SOURCE_DIR>/gocryptfs-xray ${EXTPREFIX}/bin/gocryptfs-xray
                COMMAND chmod 0755 ${EXTPREFIX}/bin/gocryptfs ${EXTPREFIX}/bin/gocryptfs-xray
                COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTPREFIX}/share/licenses/digikam-private
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-jacobsa-crypto-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-go-fuse-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-eme-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-go-gitignore-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-pflag-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-go-LICENSE
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-go-PATENTS
                        ${CMAKE_CURRENT_LIST_DIR}/gocryptfs-THIRD-PARTY-NOTICES
                        ${EXTPREFIX}/share/licenses/digikam-private

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
