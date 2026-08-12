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

#include "KmlSimpleFieldTagWriter.h"

// Qt includes

#include <QString>

// Local includes

#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerSimpleField(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataSimpleFieldType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlSimpleFieldTagWriter);

bool KmlSimpleFieldTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataSimpleField* simpleField = static_cast<const GeoDataSimpleField*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_SimpleField));
    writer.writeAttribute(QString::fromUtf8("name"), simpleField->name());
    GeoDataSimpleField::SimpleFieldType simpleFieldType = simpleField->type();
    QString type = resolveType(simpleFieldType);
    writer.writeAttribute(QString::fromUtf8("type"), type);

    QString displayName = simpleField->displayName();
    writer.writeElement(QString::fromUtf8("displayName"), displayName);

    writer.writeEndElement();

    return true;
}

QString KmlSimpleFieldTagWriter::resolveType(GeoDataSimpleField::SimpleFieldType type)
{
    switch (type)
    {
        case GeoDataSimpleField::String:
            return QString::fromUtf8("string");

        case GeoDataSimpleField::Int:
            return QString::fromUtf8("int");

        case GeoDataSimpleField::UInt:
            return QString::fromUtf8("uint");

        case GeoDataSimpleField::Short:
            return QString::fromUtf8("short");

        case GeoDataSimpleField::UShort:
            return QString::fromUtf8("ushort");

        case GeoDataSimpleField::Float:
            return QString::fromUtf8("float");

        case GeoDataSimpleField::Double:
            return QString::fromUtf8("double");

        case GeoDataSimpleField::Bool:
            return QString::fromUtf8("bool");
    }

    Q_ASSERT(false);
    return QString::fromUtf8("string");
}

} // namespace Marble
