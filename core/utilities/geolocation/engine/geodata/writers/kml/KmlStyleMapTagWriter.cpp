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

#include "KmlStyleMapTagWriter.h"

// Local includes

#include "GeoDataStyleMap.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerStyleMap(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataStyleMapType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlStyleMapTagWriter);

bool KmlStyleMapTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataStyleMap* map = static_cast<const GeoDataStyleMap*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_StyleMap));
    KmlObjectTagWriter::writeIdentifiers(writer, map);

    QMapIterator<QString, QString> iter(*map);

    while (iter.hasNext())
    {
        iter.next();
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Pair));
        writer.writeElement(QString::fromUtf8(kml::kmlTag_key), iter.key());
        writer.writeElement(QString::fromUtf8(kml::kmlTag_styleUrl), iter.value());
        writer.writeEndElement();
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
