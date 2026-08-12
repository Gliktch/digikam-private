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

#include "KmlDataTagWriter.h"

// Local includes

#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "GeoDataData.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerData(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataDataType),
                                                                      QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                          new KmlDataTagWriter());


bool KmlDataTagWriter::write(const GeoNode* node,
                             GeoWriter& writer) const
{
    const GeoDataData* data = static_cast<const GeoDataData*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Data));
    writer.writeAttribute(QString::fromUtf8("name"), data->name());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_displayName), data->displayName());
    writer.writeElement(QString::fromUtf8("value"), data->value().toString());
    writer.writeEndElement();

    return true;
}

} // namespace Marble
