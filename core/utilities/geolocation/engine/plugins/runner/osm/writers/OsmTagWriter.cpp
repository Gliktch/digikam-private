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

#include "OsmTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "OsmElementDictionary.h"
#include "MarbleGlobal.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerOsm(GeoTagWriter::QualifiedName(QString::fromUtf8(""),
                                                                     QString::fromUtf8(osm::osmTag_version06)),
                                         new OsmTagWriter());


bool OsmTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    Q_UNUSED(node);

    writer.writeStartElement(QLatin1String("osm"));
    writer.writeAttribute(QLatin1String("version"), QString::fromUtf8(osm::osmTag_version06));
    writer.writeAttribute(QLatin1String("generator"), QLatin1String("Marble ") + Marble::MARBLE_VERSION_STRING);

    return true;
}

} // namespace Marble
