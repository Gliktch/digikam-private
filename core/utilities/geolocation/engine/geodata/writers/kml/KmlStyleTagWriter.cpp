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

#include "KmlStyleTagWriter.h"

// Local includes

#include "GeoDataStyle.h"
#include "GeoDataBalloonStyle.h"
#include "GeoDataIconStyle.h"
#include "GeoDataLabelStyle.h"
#include "GeoDataLineStyle.h"
#include "GeoDataListStyle.h"
#include "GeoDataPolyStyle.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerStyle(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataStyleType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlStyleTagWriter);

bool KmlStyleTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataStyle* style = static_cast<const GeoDataStyle*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Style));
    KmlObjectTagWriter::writeIdentifiers(writer, style);

    writeElement(&style->iconStyle(), writer);
    writeElement(&style->labelStyle(), writer);
    writeElement(&style->lineStyle(), writer);
    writeElement(&style->polyStyle(), writer);
    writeElement(&style->balloonStyle(), writer);
    writeElement(&style->listStyle(), writer);

    writer.writeEndElement();

    return true;
}

} // namespace Marble
