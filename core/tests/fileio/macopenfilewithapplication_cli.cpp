/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-04
 * Description : a command line tool to open file with the default
 *               macOS application bundle.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QUrl>
#include <QList>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "dservicemenu.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "openfilewithapplication - Open file with default macOS bundle Application";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <file path>";
        return -1;
    }

    QString fname    = QString::fromUtf8(argv[1]);
    QString suffix   = QFileInfo(fname).suffix();
    QList<QUrl> list = DServiceMenu::MacApplicationForFileExtension(suffix);

    if (list.isEmpty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "No application found to open" << fname;
        return -1;
    }

    QUrl appUrl  = list.first();
    QUrl fileUrl = QUrl::fromLocalFile(fname);

    bool ret = DServiceMenu::MacOpenFilesWithApplication(QList<QUrl>() << fileUrl, appUrl);

    if (!ret)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Cannot start application" << DServiceMenu::MacApplicationBundleName(appUrl) << "to open" << fname;
        return -1;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "Application" << DServiceMenu::MacApplicationBundleName(appUrl) << "started to open" << fname;

    return 0;
}
