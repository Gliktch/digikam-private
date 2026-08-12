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

#include "KmlPolyStyleTagWriter.h"

// Local includes

#include "GeoDataPolyStyle.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLineStyle(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataPolyStyleType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlPolyStyleTagWriter);

KmlPolyStyleTagWriter::KmlPolyStyleTagWriter() : KmlColorStyleTagWriter(QString::fromUtf8(kml::kmlTag_PolyStyle))
{
}

bool KmlPolyStyleTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataPolyStyle* style = static_cast<const GeoDataPolyStyle*>(node);

    if (!style->fill())
    {
        writer.writeElement(QString::fromUtf8("fill"), QString::fromUtf8("0"));
    }

    if (!style->outline())
    {
        writer.writeElement(QString::fromUtf8("outline"), QString::fromUtf8("0"));
    }

    return true;
}

bool KmlPolyStyleTagWriter::isEmpty(const GeoNode* node) const
{
    const GeoDataPolyStyle* style = static_cast<const GeoDataPolyStyle*>(node);
    return style->fill() && style->outline();
}

} // namespace Marble
