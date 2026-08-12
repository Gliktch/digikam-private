#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

file(GLOB _icons_files *.png *.svgz)

ecm_install_icons(ICONS       ${_icons_files}
                  DESTINATION ${KDE_INSTALL_FULL_ICONDIR}
)
