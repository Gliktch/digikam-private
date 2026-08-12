/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to test ExifTool stream parsing.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QByteArray>
#include <QCoreApplication>
#include <QFile>
#include <QObject>

// Local includes

#include "digikam_debug.h"
#include "exiftoolparser.h"
#include "exiftoolprocess.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "exiftoolparserout_cli - CLI tool to check ExifTool stream parsing";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <ExifTool input stream>";
        return -1;
    }

    QFile input(QString::fromUtf8(argv[1]));

    if (!input.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Cannot open ExifTool input stream to read...";
        return -1;
    }

    QByteArray stdOut            = input.readAll();

    ExifToolParser* const parser = new ExifToolParser(qApp);

    parser->setOutputStream(ExifToolProcess::LOAD_METADATA,     // Command action ID
                            stdOut,                             // Output channel
                            QByteArray());                      // Error channel

    return 0;
}
