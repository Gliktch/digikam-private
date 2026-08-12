#
# SPDX-FileCopyrightText: 2015, Andrej Krutak, <dev at andree dot sk>
#
# SPDX-License-Identifier: BSD-3-Clause
#

file(GLOB _files *.webp)

install(FILES ${_files}
        DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/data/lut3d
)
