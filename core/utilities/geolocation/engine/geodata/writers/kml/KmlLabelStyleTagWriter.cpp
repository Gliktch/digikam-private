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

#include "KmlLabelStyleTagWriter.h"

// Local includes

#include "GeoDataLabelStyle.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLineStyle(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLabelStyleType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLabelStyleTagWriter);

KmlLabelStyleTagWriter::KmlLabelStyleTagWriter() : KmlColorStyleTagWriter(QString::fromUtf8(kml::kmlTag_LabelStyle))
{
    //nothing to do here
}

bool KmlLabelStyleTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataLabelStyle* style = static_cast<const GeoDataLabelStyle*>(node);

    writer.writeElement(QString::fromUtf8(kml::kmlTag_scale), QString::number(style->scale()));

    return true;
}

bool KmlLabelStyleTagWriter::isEmpty(const GeoNode* node) const
{
    const GeoDataLabelStyle* style = static_cast<const GeoDataLabelStyle*>(node);

    return style->scale() == 1.0;
}

QColor KmlLabelStyleTagWriter::defaultColor() const
{
    return QColor(Qt::black);
}

} // namespace Marble
