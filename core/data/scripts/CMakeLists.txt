#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

install(PROGRAMS digitaglinktree/digitaglinktree
                 cleanup_digikamdb/cleanup_digikamdb
        DESTINATION ${KDE_INSTALL_FULL_BINDIR}
)

install(FILES digitaglinktree/digitaglinktree.1
              cleanup_digikamdb/cleanup_digikamdb.1
        DESTINATION ${KDE_INSTALL_FULL_MANDIR}/man1
)

