#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

# ----------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/correlator_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

# ----------------------------------------------------------------

if(NOT WIN32)

    ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/rgparsing_utest.cpp

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  digikamcore

                  ${COMMON_TEST_LINK}
    )

endif()

# ----------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/gpsimageitem_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

# ----------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/rgtagmodel_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              libmodeltest

              ${COMMON_TEST_LINK}
)

# ----------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/simpletreemodel_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              libmodeltest

              ${COMMON_TEST_LINK}
)
