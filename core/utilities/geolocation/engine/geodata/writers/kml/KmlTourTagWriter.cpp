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

#include "KmlTourTagWriter.h"

// Local includes

#include "GeoDataTour.h"
#include "GeoDataTypes.h"
#include "GeoDataPlaylist.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerTour(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataTourType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlTourTagWriter);

bool KmlTourTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const  GeoDataTour* tour = static_cast<const GeoDataTour*>(node);

    if (tour->playlist())
    {
        writeElement(tour->playlist(), writer);
    }

    return true;
}

KmlTourTagWriter::KmlTourTagWriter() :
    KmlFeatureTagWriter(QString::fromUtf8("gx:Tour"))
{
    // nothing to do
}

} // namespace Marble
