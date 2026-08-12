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

#include "DgmlSectionTagWriter.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoWriter.h"
#include "GeoSceneSection.h"
#include "GeoSceneItem.h"
#include "GeoSceneIcon.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerSection(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneSectionType),
                                                                         QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                             new DgmlSectionTagWriter());

bool DgmlSectionTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneSection* section = static_cast<const GeoSceneSection*>(node);

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Section));
    writer.writeAttribute(QString::fromUtf8("name"),        section->name());
    writer.writeAttribute(QString::fromUtf8("checkable"),   section->checkable() ? QString::fromUtf8("true")
                                                                                 : QString::fromUtf8("false"));
    writer.writeAttribute(QString::fromUtf8("connect"),     section->connectTo());
    writer.writeAttribute(QString::fromUtf8("spacing"),     QString::number(section->spacing()));
    writer.writeElement(QString::fromUtf8(dgml::dgmlTag_Heading), section->heading());

    for (int i = 0 ; i < section->items().count() ; ++i)
    {
        const GeoSceneItem* item = section->items().at(i);
        writeElement(item, writer);
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
