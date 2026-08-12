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

#include "KmlExtendedDataTagWriter.h"

// Qt includes

#include <QHash>

// Local includes

#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "GeoDataExtendedData.h"
#include "GeoDataData.h"
#include "GeoDataSchemaData.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerExtendedData(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataExtendedDataType),
                                                                              QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                                  new KmlExtendedDataTagWriter());


bool KmlExtendedDataTagWriter::write(const GeoNode* node,
                                     GeoWriter& writer) const
{
    const GeoDataExtendedData* extended = static_cast<const GeoDataExtendedData*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ExtendedData));

    QHash< QString, GeoDataData >::const_iterator begin = extended->constBegin();
    QHash< QString, GeoDataData >::const_iterator end = extended->constEnd();

    for (QHash< QString, GeoDataData >::const_iterator i = begin; i != end; ++i)
    {
        writeElement(&i.value(), writer);
    }

    for (const GeoDataSchemaData& schemaData : extended->schemaDataList())
    {
        writeElement(&schemaData, writer);
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
