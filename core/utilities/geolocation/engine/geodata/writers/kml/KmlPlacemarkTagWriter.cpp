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

#include "KmlPlacemarkTagWriter.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataPlacemark.h"
#include "GeoDataExtendedData.h"
#include "GeoDataTimeStamp.h"
#include "GeoDataTypes.h"
#include "GeoDataPolygon.h"
#include "GeoWriter.h"
#include "GeoDataStyle.h"

namespace Marble
{

//needs to handle a specific doctype. different versions different writer classes?
//don't use the tag dictionary for tag names, because with the writer we are using
// the object type strings instead
//FIXME: USE object strings provided by idis
static GeoTagWriterRegistrar s_writerPlacemark(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataPlacemarkType),
                                                                           QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)), new KmlPlacemarkTagWriter());

bool KmlPlacemarkTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataPlacemark* placemark = static_cast<const GeoDataPlacemark*>(node);

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_styleUrl), placemark->styleUrl());

    if (placemark->styleUrl().isEmpty() && placemark->customStyle())
    {
        writeElement(placemark->customStyle().data(), writer);
    }

    if (placemark->geometry())
    {
        writeElement(placemark->geometry(), writer);
    }

    if (placemark->isBalloonVisible())
    {
        QString string;
        string.setNum(1);
        writer.writeElement(QString::fromUtf8(kml::kmlTag_nameSpaceGx22), QString::fromUtf8(kml::kmlTag_balloonVisibility), string);
    }

    return true;
}

KmlPlacemarkTagWriter::KmlPlacemarkTagWriter() :
    KmlFeatureTagWriter(QString::fromUtf8(kml::kmlTag_Placemark))
{
    // nothing to do
}

} // namespace Marble
