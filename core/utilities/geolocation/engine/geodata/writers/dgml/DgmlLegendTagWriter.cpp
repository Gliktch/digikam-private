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

#include "DgmlLegendTagWriter.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoWriter.h"
#include "GeoSceneLegend.h"
#include "GeoSceneSection.h"
#include "DgmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLegend(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneLegendType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                            new DgmlLegendTagWriter());

bool DgmlLegendTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneLegend* legend = static_cast<const GeoSceneLegend*>(node);

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Legend));

    for (int i = 0; i < legend->sections().count(); ++i)
    {
        const GeoSceneSection* section = legend->sections().at(i);
        writeElement(section, writer);
    }

    writer.writeEndElement();
    return true;
}

} // namespace Marble
