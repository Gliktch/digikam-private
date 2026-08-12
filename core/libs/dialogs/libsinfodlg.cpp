/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "libsinfodlg_p.h"

namespace Digikam
{

LibsInfoDlg::LibsInfoDlg(QWidget* const parent)
    : InfoDlg(parent),
      d      (new Private)
{
    setWindowTitle(i18nc("@title:window", "Shared Libraries and Components Information"));

    listView()->setHeaderLabels(QStringList() << QLatin1String("Properties")
                                              << QLatin1String("Value"));    // Hidden header -> no i18n
    listView()->setSortingEnabled(true);
    listView()->setRootIsDecorated(true);
    listView()->setSelectionMode(QAbstractItemView::SingleSelection);
    listView()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listView()->setAllColumnsShowFocus(true);
    listView()->setColumnCount(2);
    listView()->header()->setSectionResizeMode(QHeaderView::Stretch);
    listView()->header()->hide();

    // ---

    populateFeatures();

    // ---

    populateLibraries();

    // ---

    populateBuildTools();

    // ---

    populateManifest();

    // ---

    populateOpenCV();
}

LibsInfoDlg::~LibsInfoDlg()
{
    delete d;
}

} // namespace Digikam

#include "moc_libsinfodlg.cpp"
