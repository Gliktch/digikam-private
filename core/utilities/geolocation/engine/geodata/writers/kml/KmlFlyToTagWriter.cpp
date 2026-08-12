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

#include "KmlFlyToTagWriter.h"

// Local includes

#include "GeoDataFlyTo.h"
#include "GeoDataTypes.h"
#include "GeoDataAbstractView.h"
#include "GeoDataLookAt.h"
#include "GeoDataCamera.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerFlyTo(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataFlyToType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlFlyToTagWriter);

bool KmlFlyToTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataFlyTo* flyTo = static_cast<const GeoDataFlyTo*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceGx22), QString::fromUtf8(kml::kmlTag_FlyTo));
    writer.writeElement(QString::fromUtf8(kml::kmlTag_nameSpaceGx22), QString::fromUtf8(kml::kmlTag_duration), QString::number(flyTo->duration()));

    if (flyTo->flyToMode() == GeoDataFlyTo::Smooth)
    {
        // two values, smooth and bounce, bounce is default and can hence be omitted
        writer.writeElement(QString::fromUtf8(kml::kmlTag_nameSpaceGx22), QString::fromUtf8(kml::kmlTag_flyToMode), QString::fromUtf8("smooth"));
    }

    if (flyTo->view())
    {
        GeoDataLookAt const* lookAt = dynamic_cast<const GeoDataLookAt*>(flyTo->view());

        if (lookAt)
        {
            writeElement(lookAt, writer);
        }

        GeoDataCamera const* camera = dynamic_cast<const GeoDataCamera*>(flyTo->view());

        if (camera)
        {
            writeElement(camera, writer);
        }
    }

    writer.writeEndElement();
    return true;
}

} // namespace Marble
