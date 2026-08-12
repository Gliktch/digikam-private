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

#include "KmlTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerKml(GeoTagWriter::QualifiedName(QString::fromUtf8(""),
                                                                     QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                         new KmlTagWriter());


bool KmlTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    Q_UNUSED(node);

    writer.writeDefaultNamespace(QString::fromUtf8(kml::kmlTag_nameSpaceOgc22));
    writer.writeNamespace(QString::fromUtf8(kml::kmlTag_nameSpaceGx22), QLatin1String("gx"));
    writer.writeStartElement(QLatin1String("kml"));

    // Do not write an end element for document handlers
    return true;
}

} // namespace Marble
