/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to export metadata changes to EXV
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QFileInfo>
#include <QString>
#include <QApplication>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "dmetadatadiff_cli - CLI tool to export metadata changes to EXV";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));

    DMetadata meta;
    bool ret = meta.load(input.filePath());

    if (!ret)
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Cannot load" << meta.getFilePath();
        return -1;
    }

    meta.setImageDateTime(QDateTime::currentDateTime(), true);

    QString exvPath = input.baseName() + QLatin1String("_changes.exv");
    meta.exportChanges(exvPath);

    return 0;
}

