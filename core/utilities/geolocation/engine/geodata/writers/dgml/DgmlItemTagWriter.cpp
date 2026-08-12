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

#include "DgmlItemTagWriter.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoWriter.h"
#include "GeoSceneSection.h"
#include "GeoSceneItem.h"
#include "GeoSceneIcon.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerItem(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneItemType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                          new DgmlItemTagWriter());

bool DgmlItemTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneItem* item = static_cast<const GeoSceneItem*>(node);

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Item));
    writer.writeAttribute(QString::fromUtf8("name"), item->name());

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Icon));

    if (!item->icon()->pixmap().isEmpty())
    {
        writer.writeAttribute(QString::fromUtf8("pixmap"), item->icon()->pixmap());
    }

    else if (item->icon()->color().isValid())
    {
        writer.writeAttribute(QString::fromUtf8("color"), item->icon()->color().name());
    }

    writer.writeEndElement();
    writer.writeElement(QString::fromUtf8("text"), item->text());

    writer.writeEndElement();
    return true;
}

} // namespace Marble
