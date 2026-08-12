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

#include "KmlAnimatedUpdateTagWriter.h"

// Local includes

#include "GeoDataAnimatedUpdate.h"
#include "GeoDataTypes.h"
#include "GeoDataUpdate.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerAnimatedUpdate(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataAnimatedUpdateType),
                                                                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)), new KmlAnimatedUpdateTagWriter);

bool KmlAnimatedUpdateTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    Q_ASSERT(dynamic_cast<const GeoDataAnimatedUpdate*>(node));
    const GeoDataAnimatedUpdate* animUpdate = static_cast<const GeoDataAnimatedUpdate*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceGx22), QString::fromUtf8(kml::kmlTag_AnimatedUpdate));
    KmlObjectTagWriter::writeIdentifiers(writer, animUpdate);
    writer.writeOptionalElement(QString::fromUtf8("gx:duration"), animUpdate->duration(), 0.0);

    if (animUpdate->update())
    {
        GeoDataUpdate const* update = dynamic_cast<const GeoDataUpdate*>(animUpdate->update());

        if (update)
        {
            writeElement(update, writer);
        }
    }

    writer.writeOptionalElement(QString::fromUtf8("gx:delayedStart"), animUpdate->delayedStart(), 0.0);
    writer.writeEndElement();
    return true;
}

} // namespace Marble
