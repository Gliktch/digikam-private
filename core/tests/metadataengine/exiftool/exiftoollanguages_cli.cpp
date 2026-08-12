/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to get list of languages available for translations
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
    ExifToolParser::ExifToolData parsed;
    QStringList lst;

    if (parser->translationsList())
    {
        parsed = parser->currentData();
        lst    = parsed.find(QLatin1String("TRANSLATIONS_LIST")).value()[0].toStringList();
        qCDebug(DIGIKAM_TESTS_LOG) << "Available translations:";

        for (int i = 0 ; i < lst.size() ; ++i)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << lst[i];
        }
    }

    return 0;
}
