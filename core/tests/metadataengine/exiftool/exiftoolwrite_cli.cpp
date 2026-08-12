/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to write metadata with ExifTool
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
        qCDebug(DIGIKAM_TESTS_LOG) << "exiftoolwrite_cli - CLI tool to write metadata with ExifTool in image";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image to patch>";
        return -1;
    }

    MetaEngine::initializeExiv2();
    DPluginLoader::instance()->init();

    // Write all metadata to an empty JPG file.

    DImg file(1, 1, false);
    file.save(QString::fromUtf8(argv[1]), DImg::JPEG);

    // Create ExifTool parser instance.

    ExifToolParser* const parser = new ExifToolParser(qApp);

    ExifToolParser::ExifToolData newTags;
    newTags.insert(QLatin1String("EXIF:ImageDescription"),
                   QVariantList() << QString()      // not used
                                  << QString::fromUtf8("J'ai reçu cette photo par la poste en Février"));
    newTags.insert(QLatin1String("xmp:city"),
                   QVariantList() << QString()      // not used
                                  << QString::fromUtf8("Paris"));

    // Read metadata from the file. Start ExifToolParser

    if (!parser->applyChanges(QString::fromUtf8(argv[1]), newTags))
    {
        return -1;
    }

    return 0;
}
