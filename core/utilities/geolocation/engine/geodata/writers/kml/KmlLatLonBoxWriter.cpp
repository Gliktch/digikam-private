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

#include "KmlLatLonBoxWriter.h"

// Local includes

#include "GeoDataLatLonBox.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLookAt(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLatLonBoxType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLatLonBoxWriter);

bool KmlLatLonBoxWriter::write(const GeoNode* node,
                               GeoWriter& writer) const
{
    const GeoDataLatLonBox* lat_lon_box =
        static_cast<const GeoDataLatLonBox*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_LatLonBox));
    KmlObjectTagWriter::writeIdentifiers(writer, lat_lon_box);

    writer.writeTextElement(QString::fromUtf8("north"),
                            QString::number(lat_lon_box->north(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8("south"),
                            QString::number(lat_lon_box->south(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8("east"),
                            QString::number(lat_lon_box->east(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8("west"),
                            QString::number(lat_lon_box->west(GeoDataCoordinates::Degree)));
    writer.writeOptionalElement(QString::fromUtf8("rotation"),
                                QString::number(lat_lon_box->rotation(GeoDataCoordinates::Degree)), QString::fromUtf8("0"));

    writer.writeEndElement();

    return true;
}

} // namespace Marble
