/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "GpsbabelRunner.h"

// Qt includes

#include <QFile>
#include <QProcess>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QDir>
#include <QMap>

// Local includes

#include "GeoDataParser.h"
#include "GeoDataDocument.h"
#include "digikam_debug.h"

namespace Marble
{

GpsbabelRunner::GpsbabelRunner(QObject* parent)
    : ParsingRunner(parent)
{
}

GeoDataDocument* GpsbabelRunner::parseFile(const QString& fileName, DocumentRole role, QString& error)
{
    // Check and see if the file exists

    if (!QFileInfo::exists(fileName))
    {
        error = QStringLiteral("File %1 does not exist").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    // Inspect the filename suffix

    QString const fileSuffix = QFileInfo(fileName).suffix();

    // Determine if fileName suffix is supported by this plugin

    QMap<QString, QString> fileTypes;
    fileTypes[QLatin1String("nmea")]     = QLatin1String("nmea");
    fileTypes[QLatin1String("igc")]      = QLatin1String("igc");
    fileTypes[QLatin1String("tiger")]    = QLatin1String("tiger");
    fileTypes[QLatin1String("ov2")]      = QLatin1String("tomtom");
    fileTypes[QLatin1String("garmin")]   = QLatin1String("garmin_txt");
    fileTypes[QLatin1String("magellan")] = QLatin1String("magellan");
    fileTypes[QLatin1String("csv")]      = QLatin1String("csv");
    QString const inputFileType = fileTypes[fileSuffix];

    if (inputFileType.isEmpty())
    {
        error = QStringLiteral("Unsupported file extension for").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    // Set up temporary file to hold output KML from gpsbabel executable

    QTemporaryFile tempKmlFile(QDir::tempPath() + QLatin1String("/marble-gpsbabel-XXXXXX.kml"));

    if (!tempKmlFile.open())
    {
        qCWarning(DIGIKAM_GEOENGINE_LOG) << "Cannot open KML temp file" << tempKmlFile.fileName();

        return nullptr;
    }

    QFile kmlFile(tempKmlFile.fileName());

    // Set up gpsbabel command line

    const QString command  = QLatin1String("gpsbabel");
    const QStringList args = QStringList()
                             << QLatin1String("-i")
                             << inputFileType
                             << QLatin1String("-f")
                             << fileName
                             << QLatin1String("-o")
                             << QLatin1String("kml")
                             << QLatin1String("-F")
                             << tempKmlFile.fileName()
                             ;

    // Execute gpsbabel to parse the input file

    int const exitStatus = QProcess::execute(command, args);

    if (exitStatus == 0)
    {
        if (!kmlFile.open(QIODevice::ReadWrite))
        {
            qCWarning(DIGIKAM_GEOENGINE_LOG) << "Cannot open KML file" << kmlFile.fileName();

            return nullptr;
        }

        GeoDataParser parser(GeoData_KML);
        parser.read(&kmlFile);
        GeoDataDocument* const document = dynamic_cast<GeoDataDocument*>(parser.releaseDocument());

        if (!document)
        {
            error = parser.errorString();
            qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

            return nullptr;
        }

        document->setDocumentRole(role);

        return document;
    }
    else
    {
        error = QStringLiteral("Gpsbabel returned error code %1").arg(exitStatus);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }
}

} // namespace Marble

#include "moc_GpsbabelRunner.cpp"
