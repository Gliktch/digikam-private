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

#include "DgmlDocumentTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "GeoSceneMap.h"
#include "GeoSceneSettings.h"
#include "GeoSceneLegend.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerDocument(GeoTagWriter::QualifiedName(QString::fromUtf8("GeoSceneDocument"), QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                              new DgmlDocumentTagWriter());

bool DgmlDocumentTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneDocument* document = static_cast<const GeoSceneDocument*>(node);

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Document));

    const GeoSceneHead* head = document->head();
    writeElement(head, writer);

    const GeoSceneMap* map = document->map() ;
    writeElement(map, writer);

    const GeoSceneSettings* settings = document->settings();
    writeElement(settings, writer);

    const GeoSceneLegend* legend = document->legend();
    writeElement(legend, writer);

    writer.writeEndDocument();
    return true;
}

} // namespace Marble
