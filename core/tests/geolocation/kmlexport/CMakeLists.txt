#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(${CMAKE_SOURCE_DIR}/core/dplugins/generic/metadata/geolocationedit/kmlexport)

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/geoparsing_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)
