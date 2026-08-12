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

#include "DgmlVectorTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "GeoSceneVector.h"
#include "GeoSceneTypes.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerVector(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneVectorType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)), new DgmlVectorTagWriter());

bool DgmlVectorTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneVector* vector = static_cast<const GeoSceneVector*>(node);
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Vector));
    writer.writeAttribute(QLatin1String("name"), vector->name());
    writer.writeAttribute(QLatin1String("feature"), vector->feature());

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_SourceFile));
    writer.writeAttribute(QLatin1String("format"), vector->fileFormat());
    writer.writeCharacters(vector->sourceFile());
    writer.writeEndElement();

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Pen));
    writer.writeAttribute(QLatin1String("color"), vector->pen().color().name());
    writer.writeEndElement();

    writer.writeEndElement();
    return true;
}

} // namespace Marble
