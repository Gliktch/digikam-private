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

#include "CacheRunner.h"

// Qt includes

#include <QFile>
#include <QDataStream>
#include <QSet>

// Local includes

#include "GeoDataDocument.h"
#include "GeoDataExtendedData.h"
#include "GeoDataData.h"
#include "GeoDataPlacemark.h"
#include "digikam_debug.h"

namespace Marble
{

const quint32 MarbleMagicNumber = 0x31415926;

CacheRunner::CacheRunner(QObject* const parent)
    : ParsingRunner(parent)
{
}

CacheRunner::~CacheRunner()
{
}

GeoDataDocument* CacheRunner::parseFile(const QString& fileName, DocumentRole role, QString& error)
{
    QFile file(fileName);

    if (!file.exists())
    {
        error = QStringLiteral("File %1 does not exist").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        error = QStringLiteral("File %1 cannot be open").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    QDataStream in(&file);

    // Read and check the header

    quint32 magic;
    in >> magic;

    if (magic != MarbleMagicNumber)
    {
        error = QStringLiteral("File %1 is not a recognized format").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    // Read the version
    qint32 version;
    in >> version;

    if (version < 015)
    {
        error = QStringLiteral("Bad cache file %1: Version %2 is too old, need 15 or later").arg(fileName).arg(version);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    /*
      if (version > 002) {
      qCDebug(DIGIKAM_GEOENGINE_LOG) << "Bad file - too new!";
      return;
      }
    */

    GeoDataDocument* const document = new GeoDataDocument();
    document->setDocumentRole(role);

    in.setVersion(QDataStream::Qt_4_2);

    // Read the data itself
    // Use double to provide a single cache file format across architectures

    double   lon;
    double   lat;
    double   alt;
    double   area;

    QString  tmpstr;
    qint64   tmpint64;
    qint8    tmpint8;
    qint16   tmpint16;

    // share string data on the heap at least for this file
    QSet<QString> stringPool;
    const QString gmtId = QStringLiteral("gmt");
    const QString dstId = QStringLiteral("dst");

    while (!in.atEnd())
    {
        GeoDataPlacemark* mark = new GeoDataPlacemark;
        in >> tmpstr;
        tmpstr = *stringPool.insert(tmpstr);
        mark->setName(tmpstr);
        in >> lon >> lat >> alt;
        mark->setCoordinate((qreal)(lon), (qreal)(lat), (qreal)(alt));
        in >> tmpstr;
        tmpstr = *stringPool.insert(tmpstr);
        mark->setRole(tmpstr);
        in >> tmpstr;
        tmpstr = *stringPool.insert(tmpstr);
        mark->setDescription(tmpstr);
        in >> tmpstr;
        tmpstr = *stringPool.insert(tmpstr);
        mark->setCountryCode(tmpstr);
        in >> tmpstr;
        tmpstr = *stringPool.insert(tmpstr);
        mark->setState(tmpstr);
        in >> area;
        mark->setArea((qreal)(area));
        in >> tmpint64;
        mark->setPopulation(tmpint64);
        in >> tmpint16;
        mark->extendedData().addValue(GeoDataData(gmtId, int(tmpint16)));
        in >> tmpint8;
        mark->extendedData().addValue(GeoDataData(dstId, int(tmpint8)));

        document->append(mark);
    }

    document->setFileName(fileName);

    file.close();

    return document;
}

} // namespace Marble

#include "moc_CacheRunner.cpp"
