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

void LibsInfoDlg::populateManifest()
{
    /**
     * NOTE: MANIFEST.txt is a text file generated with the bundles and listing all git revisions of rolling release components.
     *       One section title start with '+'.
     *       All component revisions are listed below line by line with the name and the revision separated by ':'.
     *       More than one section can be listed in manifest.
     */

    const QString gitRevs = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                   QLatin1String("MANIFEST.txt"));

    if (!gitRevs.isEmpty() && QFile::exists(gitRevs))
    {
        QFile file(gitRevs);

        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        qCDebug(DIGIKAM_WIDGETS_LOG) << "Git revisions manifest file found:" << gitRevs;

        QTreeWidgetItem* const manifestHead = new QTreeWidgetItem(listView(), QStringList() << i18nc("@item: component info", "Manifests"));
        listView()->addTopLevelItem(manifestHead);

        QTextStream in(&file);
        QTreeWidgetItem* manifestEntry = nullptr;

#ifndef __clang_analyzer__

        // Disable false-positive memory leak reported by scan-build with QTreeWidgetItem.

        while (!in.atEnd())
        {
            QString line = in.readLine();

            if (line.isEmpty())
            {
                continue;
            }

            if      (line.startsWith(QLatin1Char('+')))
            {
                manifestEntry = new QTreeWidgetItem(manifestHead, QStringList() << line.remove(QLatin1Char('+')));
            }
            else if (manifestEntry)
            {
                new QTreeWidgetItem(manifestEntry, line.split(QLatin1Char(':')));
            }
        }

#endif

        file.close();
    }
}

} // namespace Digikam
