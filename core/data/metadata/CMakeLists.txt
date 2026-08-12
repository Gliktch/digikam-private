#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/topicset.iptc-subjectcode.xml
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/metadata
)

file(GLOB dkampfiles *.dkamp)

install(FILES  ${dkampfiles}
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/metadata
)
