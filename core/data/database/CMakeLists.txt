#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# NOTE: the DBCORECONFIG_XML_VERSION ID is configured in the toplevel CMakeLists.txt,
configure_file(dbconfig.xml.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml
              ${CMAKE_CURRENT_SOURCE_DIR}/mysql-global.conf
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/database
)
