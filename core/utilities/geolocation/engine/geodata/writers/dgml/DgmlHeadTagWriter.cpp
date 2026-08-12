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

#include "DgmlHeadTagWriter.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoWriter.h"
#include "GeoSceneHead.h"
#include "GeoSceneIcon.h"
#include "GeoSceneZoom.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerHead(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneHeadType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                          new DgmlHeadTagWriter());

bool DgmlHeadTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneHead* head = static_cast<const GeoSceneHead*>(node);
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Head));
    writer.writeElement(QString::fromUtf8("name"), head->name());
    writer.writeElement(QString::fromUtf8("target"), head->target());
    writer.writeElement(QString::fromUtf8("theme"), head->theme());
    writer.writeElement(QString::fromUtf8("visible"), head->visible() ? QString::fromUtf8("true") : QString::fromUtf8("false"));
    writer.writeStartElement(QString::fromUtf8("description"));
    writer.writeCDATA(head->description());
    writer.writeEndElement();

    const GeoSceneIcon& icon = static_cast<const GeoSceneIcon&>(*head->icon());
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Icon));
    writer.writeAttribute(QString::fromUtf8("pixmap"), icon.pixmap());
    writer.writeEndElement();

    const GeoSceneZoom& zoom = static_cast<const GeoSceneZoom&>(*head->zoom());
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Zoom));
    writer.writeElement(QString::fromUtf8("discrete"), zoom.discrete() ? QString::fromUtf8("true") : QString::fromUtf8("false"));
    writer.writeTextElement(QString::fromUtf8("minimum"), QString::number(zoom.minimum()));
    writer.writeTextElement(QString::fromUtf8("maximum"), QString::number(zoom.maximum()));
    writer.writeEndElement();

    writer.writeEndElement();

    return true;
}

} // namespace Marble
