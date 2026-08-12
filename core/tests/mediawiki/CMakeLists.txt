#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(${CMAKE_SOURCE_DIR}/core/dplugins/generic/webservices/mediawiki/backend)

add_subdirectory(examples)

add_library(libfakeserver STATIC ${CMAKE_CURRENT_SOURCE_DIR}/fakeserver/fakeserver.cpp)

target_link_libraries(libfakeserver ${COMMON_TEST_LINK})

# ----------------------------------------------------------------------------------

macro(mediawiki_executable_tests)

    foreach(_testname ${ARGN})

        ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/${_testname}.cpp

                      NAME_PREFIX

                      "digikam-"

                      LINK_LIBRARIES

                      digikamcore
                      mediawikibackend
                      libfakeserver

                      ${COMMON_TEST_LINK}
        )

    endforeach()

endmacro()

mediawiki_executable_tests(
    edit_utest
    #login_utest
    logout_utest
    mediawiki_utest
    parse_utest
    #queryimageinfo_utest
    queryimages_utest
    queryinfo_utest
    queryrevision_utest
    querysiteinfogeneral_utest
    querysiteinfousergroups_utest
    upload_utest
)
