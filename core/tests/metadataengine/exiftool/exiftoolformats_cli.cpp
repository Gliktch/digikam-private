/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to get list of files format supported by ExifTool
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QCoreApplication>
#include <QVariant>

// Local includes

#include "digikam_debug.h"
#include "exiftoolparser.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Create ExifTool parser instance.

    ExifToolParser* const parser = new ExifToolParser(qApp);
    QString ext;
    QString desc;
    ExifToolParser::ExifToolData parsed;
    QStringList lst;

    if (parser->readableFormats())
    {
        parsed = parser->currentData();
        lst    = parsed.find(QLatin1String("READ_FORMATS")).value()[0].toStringList();
        qCDebug(DIGIKAM_TESTS_LOG) << "Readable file formats:";

        for (int i = 0 ; i < lst.size() ; i += 2)
        {
            ext  = lst[i];
            desc = lst[i + 1];
            qCDebug(DIGIKAM_TESTS_LOG) << ext << desc;
        }
    }

    if (parser->writableFormats())
    {
        parsed = parser->currentData();
        lst    = parsed.find(QLatin1String("WRITE_FORMATS")).value()[0].toStringList();
        qCDebug(DIGIKAM_TESTS_LOG) << "Writable file formats:";

        for (int i = 0 ; i < lst.size() ; i += 2)
        {
            ext  = lst[i];
            desc = lst[i + 1];
            qCDebug(DIGIKAM_TESTS_LOG) << ext << desc;
        }
    }

    return 0;
}
