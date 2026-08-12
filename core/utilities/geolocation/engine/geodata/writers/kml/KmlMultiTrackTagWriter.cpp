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

#include "KmlMultiTrackTagWriter.h"

// Local includes

#include "GeoDataMultiTrack.h"
#include "GeoDataTrack.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerMultiTrack(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataMultiTrackType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlMultiTrackTagWriter);

bool KmlMultiTrackTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataMultiTrack* geometry = static_cast<const GeoDataMultiTrack*>(node);

    writer.writeStartElement(QString::fromUtf8("gx:MultiTrack"));
    KmlObjectTagWriter::writeIdentifiers(writer, geometry);

    for (int i = 0; i < geometry->size(); ++i)
    {
        writeElement(&geometry->at(i), writer);
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
