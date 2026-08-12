/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to export ExifTool metadata
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCoreApplication>
#include <QVariant>
#include <QObject>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "dpluginloader.h"
#include "metaengine.h"
#include "exiftoolparser.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "exiftoolexport_cli - CLI tool to export ExifTool metadata from image";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();
    DPluginLoader::instance()->init();

    // Create ExifTool parser instance.

    ExifToolParser* const parser = new ExifToolParser(qApp);

    // Read metadata from the file. Start ExifToolParser

    if (!parser->loadChunk(QString::fromUtf8(argv[1])))
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Metadata chunk cannot be loaded";
        return -1;
    }

    ExifToolParser::ExifToolData chunk = parser->currentData();

    qCDebug(DIGIKAM_TESTS_LOG) << "Metadata chunk loaded";

    ExifToolParser::ExifToolData::iterator it = chunk.find(QLatin1String("EXV"));

    if (it == chunk.end())
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Metadata chunk is empty";
        return -1;
    }

    QVariantList varLst = it.value();

    MetaEngine meta;

    QByteArray exv = varLst[0].toByteArray();

    if (!exv.isEmpty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "EXV chunk size" << exv.size();
        meta.loadFromData(exv);
    }

    QFile ef(QLatin1String("output.exv"));

    if (!ef.open(QIODevice::WriteOnly))
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Cannot open target EXV file";
        return -1;
    }

    ef.write(exv);
    ef.close();

    DImg file(1, 1, false);
    file.setMetadata(meta.data());
    file.save(QLatin1String("output.jpg"), DImg::JPEG);

    return 0;
}
