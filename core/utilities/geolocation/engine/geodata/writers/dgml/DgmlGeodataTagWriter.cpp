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

#include "DgmlGeodataTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "GeoSceneGeodata.h"
#include "GeoSceneTypes.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerGeodata(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneGeodataType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)), new DgmlGeodataTagWriter());

bool DgmlGeodataTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneGeodata* geodata = static_cast<const GeoSceneGeodata*>(node);
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Geodata));
    writer.writeAttribute(QString::fromUtf8("name"), geodata->name());

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_SourceFile));
    writer.writeCharacters(geodata->sourceFile());
    writer.writeEndElement();

    writer.writeEndElement();
    return true;
}

} // namespace Marble
