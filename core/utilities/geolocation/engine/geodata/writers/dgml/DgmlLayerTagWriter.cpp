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

#include "DgmlLayerTagWriter.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoWriter.h"
#include "GeoSceneAbstractDataset.h"
#include "GeoSceneLayer.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLayer(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneLayerType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                           new DgmlLayerTagWriter());

bool DgmlLayerTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneLayer* layer = static_cast<const GeoSceneLayer*>(node);
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Layer));
    writer.writeAttribute(QString::fromUtf8("name"), layer->name());
    writer.writeAttribute(QString::fromUtf8("backend"), layer->backend());

    if (!layer->role().isEmpty())
    {
        writer.writeAttribute(QString::fromUtf8("role"), layer->role());
    }

    for (int i = 0; i < layer->datasets().count(); ++i)
    {
        writeElement(layer->datasets().at(i), writer);
    }

    writer.writeEndElement();
    return true;
}

} // namespace Marble
