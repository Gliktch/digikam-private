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

#include "KmlCameraTagWriter.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataCamera.h"
#include "GeoDataTypes.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataTimeStamp.h"
#include "GeoWriter.h"
#include "KmlGroundOverlayWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerCamera(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataCameraType),
                                                                        QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                            new KmlCameraTagWriter());


bool KmlCameraTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataCamera* camera = static_cast<const GeoDataCamera*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Camera));
    KmlObjectTagWriter::writeIdentifiers(writer, camera);

    if (camera->timeStamp().when().isValid())
    {
        writer.writeStartElement(QString::fromUtf8("gx:TimeStamp"));
        writer.writeElement(QString::fromUtf8("when"), camera->timeStamp().when().toString(Qt::ISODate));
        writer.writeEndElement();
    }

    if (camera->timeSpan().isValid())
    {
        writer.writeStartElement(QString::fromUtf8("gx:TimeSpan"));

        if (camera->timeSpan().begin().when().isValid())
        {
            writer.writeElement(QString::fromUtf8("begin"), camera->timeSpan().begin().when().toString(Qt::ISODate));
        }

        if (camera->timeSpan().end().when().isValid())
        {
            writer.writeElement(QString::fromUtf8("end"), camera->timeSpan().end().when().toString(Qt::ISODate));
        }

        writer.writeEndElement();
    }

    if (camera->longitude() != 0.0)
    {
        writer.writeElement(QString::fromUtf8("longitude"), QString::number(camera->longitude(GeoDataCoordinates::Degree), 'f', 10));
    }

    if (camera->latitude() != 0.0)
    {
        writer.writeElement(QString::fromUtf8("latitude"), QString::number(camera->latitude(GeoDataCoordinates::Degree), 'f', 10));
    }

    writer.writeOptionalElement(QString::fromUtf8("altitude"), camera->altitude());
    writer.writeOptionalElement(QString::fromUtf8("roll"), camera->roll());
    writer.writeOptionalElement(QString::fromUtf8("tilt"), camera->tilt());
    writer.writeOptionalElement(QString::fromUtf8("heading"), camera->heading());
    KmlGroundOverlayWriter::writeAltitudeMode(writer, camera->altitudeMode());

    writer.writeEndElement();

    return true;
}

} // namespace Marble
