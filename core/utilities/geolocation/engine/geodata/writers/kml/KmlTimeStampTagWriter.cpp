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

#include "KmlTimeStampTagWriter.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataTimeStamp.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerTimeStamp(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataTimeStampType),
                                                                           QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                               new KmlTimeStampTagWriter());


bool KmlTimeStampTagWriter::write(const GeoNode* node,
                                  GeoWriter& writer) const
{
    const GeoDataTimeStamp* timestamp = static_cast<const GeoDataTimeStamp*>(node);

    if (timestamp->when().isValid())
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_TimeStamp));
        KmlObjectTagWriter::writeIdentifiers(writer, timestamp);

        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_when));
        writer.writeCharacters(toString(*timestamp));
        writer.writeEndElement();

        writer.writeEndElement();
    }

    return true;
}

QString KmlTimeStampTagWriter::toString(const GeoDataTimeStamp& timestamp)
{
    switch (timestamp.resolution())
    {
        case GeoDataTimeStamp::SecondResolution:
            return timestamp.when().toString(Qt::ISODate);

        case GeoDataTimeStamp::DayResolution:
            return timestamp.when().toString(QString::fromUtf8("yyyy-MM-dd"));

        case GeoDataTimeStamp::MonthResolution:
            return timestamp.when().toString(QString::fromUtf8("yyyy-MM"));

        case GeoDataTimeStamp::YearResolution:
            return timestamp.when().toString(QString::fromUtf8("yyyy"));
    }

    Q_ASSERT(false && "not reachable");
    return QString();
}

} // namespace Marble
