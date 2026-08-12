#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.colors")

install(FILES ${files}
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/colorschemes
)
