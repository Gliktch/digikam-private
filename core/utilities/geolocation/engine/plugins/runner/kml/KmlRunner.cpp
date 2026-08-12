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

#include "KmlRunner.h"

// Qt includes

#include <QBuffer>
#include <QFile>
#include <QFileInfo>

// Local includes

#include "GeoDataDocument.h"
#include "KmlParser.h"
#include "MarbleZipReader.h"
#include "digikam_debug.h"

namespace Marble
{

KmlRunner::KmlRunner(QObject* parent) :
    ParsingRunner(parent)
{
}

KmlRunner::~KmlRunner()
{
}

GeoDataDocument* KmlRunner::parseFile(const QString& fileName, DocumentRole role, QString& error)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly))
    {
        error = QStringLiteral("Cannot open file %1").arg(fileName);
        qCDebug(DIGIKAM_GEOENGINE_LOG) << error;
        return nullptr;
    }

    QBuffer buffer;
    QIODevice* device = nullptr;

    if (fileName.toLower().endsWith(QLatin1String(".kmz")))
    {
        MarbleZipReader zipReader(&file);

        QStringList kmlFiles;

        for (const MarbleZipReader::FileInfo& zipFileInfo : zipReader.fileInfoList())
        {
            if (zipFileInfo.filePath.toLower().endsWith(QLatin1String(".kml")))
            {
                kmlFiles.append(zipFileInfo.filePath);
            }
        }

        if (kmlFiles.empty())
        {
            error = QStringLiteral("File %1 does not contain any KML files").arg(fileName);
            qCDebug(DIGIKAM_GEOENGINE_LOG) << error;
            return nullptr;
        }

        else if (kmlFiles.size() > 1)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << QStringLiteral("File %1 contains multiple KML files").arg(fileName);
        }

        QByteArray const data = zipReader.fileData(kmlFiles[0]);
        buffer.setData(data);
        buffer.open(QBuffer::ReadOnly);
        device = &buffer;
    }

    else
    {
        device = &file;
    }

    KmlParser parser;

    if (!parser.read(device))
    {
        error = parser.errorString();
        qCDebug(DIGIKAM_GEOENGINE_LOG) << error;
        return nullptr;
    }

    GeoDocument* document = parser.releaseDocument();
    Q_ASSERT(document);
    GeoDataDocument* doc = static_cast<GeoDataDocument*>(document);
    doc->setDocumentRole(role);
    doc->setFileName(fileName);

    file.close();
    return doc;
}

} // namespace Marble

#include "moc_KmlRunner.cpp"
