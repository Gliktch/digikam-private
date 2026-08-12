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

#include "KmlLookAtTagWriter.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataLookAt.h"
#include "GeoDataTypes.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataTimeStamp.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlGroundOverlayWriter.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLookAt(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLookAtType),
                                                                        QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                            new KmlLookAtTagWriter());


bool KmlLookAtTagWriter::write(const GeoNode* node,
                               GeoWriter& writer) const
{
    const GeoDataLookAt* lookAt = static_cast<const GeoDataLookAt*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_LookAt));
    KmlObjectTagWriter::writeIdentifiers(writer, lookAt);

    if (lookAt->timeStamp().when().isValid())
    {
        writer.writeStartElement(QString::fromUtf8("gx:TimeStamp"));
        writer.writeElement(QString::fromUtf8("when"), lookAt->timeStamp().when().toString(Qt::ISODate));
        writer.writeEndElement();
    }

    if (lookAt->timeSpan().isValid())
    {
        writer.writeStartElement(QString::fromUtf8("gx:TimeSpan"));

        if (lookAt->timeSpan().begin().when().isValid())
        {
            writer.writeElement(QString::fromUtf8("begin"), lookAt->timeSpan().begin().when().toString(Qt::ISODate));
        }

        if (lookAt->timeSpan().end().when().isValid())
        {
            writer.writeElement(QString::fromUtf8("end"), lookAt->timeSpan().end().when().toString(Qt::ISODate));
        }

        writer.writeEndElement();
    }

    writer.writeOptionalElement(QString::fromUtf8("longitude"), QString::number(lookAt->longitude(GeoDataCoordinates::Degree), 'f', 10));
    writer.writeOptionalElement(QString::fromUtf8("latitude"), QString::number(lookAt->latitude(GeoDataCoordinates::Degree), 'f', 10));
    writer.writeOptionalElement(QString::fromUtf8("altitude"), QString::number(lookAt->altitude(), 'f', 10));
    writer.writeOptionalElement(QString::fromUtf8("range"), QString::number(lookAt->range(), 'f', 10));
    KmlGroundOverlayWriter::writeAltitudeMode(writer, lookAt->altitudeMode());

    writer.writeEndElement();

    return true;
}

} // namespace Marble
