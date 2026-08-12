#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(LibExiv2 OPTIONAL_COMPONENTS)

if(LibExiv2_FOUND AND ("${LibExiv2_VERSION}" VERSION_GREATER "0.27.99"))

    find_package(LibExiv2 REQUIRED)
    set(EXIV2_PACKAGE_NAME "LibExiv2")

else()

    set(LibExiv2_FOUND FALSE)
    find_package(exiv2 REQUIRED)
    set(EXIV2_PACKAGE_NAME "exiv2")

endif()

set_package_properties(${EXIV2_PACKAGE_NAME} PROPERTIES
                       URL                   "https://www.exiv2.org"
                       TYPE                  RECOMMENDED
                       PURPOSE               "Library to manage image metadata (Required to build digiKam)"
)

if(LibExiv2_FOUND)

    if("${LibExiv2_VERSION}" VERSION_LESS ${EXIV2_MIN_VERSION})

        message(FATAL_ERROR "Exiv2 version is too old (${LibExiv2_VERSION})! Minimal version required:${EXIV2_MIN_VERSION}.")

    endif()

else()

    if("${exiv2_VERSION}" VERSION_LESS ${EXIV2_MIN_VERSION})

        message(FATAL_ERROR "Exiv2 version is too old (${exiv2_VERSION})! Minimal version required:${EXIV2_MIN_VERSION}.")

    endif()

endif()

if(("${exiv2_VERSION}" VERSION_GREATER "0.27.99") OR ("${LibExiv2_VERSION}" VERSION_GREATER "0.27.99"))

    set(EXIV2_CXX_STANDARD        17)
    set(EXIV2_AS_CONST_DEFINITION -DEXIV2_AS_CONST=std::as_const)

else()

    set(EXIV2_CXX_STANDARD        11)
    set(EXIV2_AS_CONST_DEFINITION -DEXIV2_AS_CONST=qAsConst)

endif()
