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

#include "KmlModelTagWriter.h"

// Local includes

#include "GeoDataModel.h"
#include "GeoDataAlias.h"
#include "GeoDataTypes.h"
#include "GeoDataLocation.h"
#include "GeoDataOrientation.h"
#include "GeoDataResourceMap.h"
#include "GeoDataScale.h"
#include "GeoWriter.h"
#include "KmlGroundOverlayWriter.h"
#include "KmlLinkTagWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerModel(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataModelType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlModelTagWriter);

bool KmlModelTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataModel* model = static_cast<const GeoDataModel*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Model));
    KmlObjectTagWriter::writeIdentifiers(writer, model);

    KmlGroundOverlayWriter::writeAltitudeMode(writer, model->altitudeMode());

    const GeoDataLocation location = model->location() ;

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Location));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_longitude), QString::number(location.longitude(GeoDataCoordinates::Degree)), QString::fromUtf8("0"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_latitude), QString::number(location.latitude(GeoDataCoordinates::Degree)), QString::fromUtf8("0"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_altitude), QString::number(location.altitude()), QString::fromUtf8("0"));

    writer.writeEndElement();

    const GeoDataOrientation orientation = model->orientation();

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Orientation));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_heading), QString::number(orientation.heading()), QString::fromUtf8("0"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_tilt), QString::number(orientation.tilt()), QString::fromUtf8("0"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_roll), QString::number(orientation.roll()), QString::fromUtf8("0"));

    writer.writeEndElement();

    const GeoDataScale scale = model->scale() ;

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Scale));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_x), QString::number(scale.x()), QString::fromUtf8("1"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_y), QString::number(scale.y()), QString::fromUtf8("1"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_z), QString::number(scale.z()), QString::fromUtf8("1"));

    writer.writeEndElement();

    writeElement(&model->link(), writer);

    const GeoDataResourceMap map = model->resourceMap() ;

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ResourceMap));

    const GeoDataAlias alias = map.alias() ;
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Alias));

    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_targetHref), alias.targetHref());
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_sourceHref), alias.sourceHref());

    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();
    return true;
}

} // namespace Marble
