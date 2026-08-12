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

#include "KmlLineStyleTagWriter.h"

// Local includes

#include "GeoDataLineStyle.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLineStyle(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLineStyleType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLineStyleTagWriter);

KmlLineStyleTagWriter::KmlLineStyleTagWriter() : KmlColorStyleTagWriter(QString::fromUtf8(kml::kmlTag_LineStyle))
{
    // nothing to do
}

bool KmlLineStyleTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataLineStyle* style = static_cast<const GeoDataLineStyle*>(node);
    writer.writeOptionalElement(QString::fromUtf8("width"), style->width(), 1.0f);
    return true;
}

bool KmlLineStyleTagWriter::isEmpty(const GeoNode* node) const
{
    const GeoDataLineStyle* style = static_cast<const GeoDataLineStyle*>(node);
    return style->width() == 1.0;
}

} // namespace Marble
