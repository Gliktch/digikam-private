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

#include "KmlDocumentTagWriter.h"

// Qt includes

#include <QVector>

// Local includes

#include "GeoDocument.h"
#include "GeoDataDocument.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "GeoDataStyle.h"
#include "GeoDataStyleMap.h"
#include "GeoDataExtendedData.h"
#include "GeoDataTimeStamp.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataSchema.h"
#include "GeoDataTypes.h"
#include "digikam_debug.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerDocument(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataDocumentType),
                                                                          QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                              new KmlDocumentTagWriter());

KmlDocumentTagWriter::KmlDocumentTagWriter() :
    KmlFeatureTagWriter(QString::fromUtf8(kml::kmlTag_Document))
{
    // nothing to do
}

bool KmlDocumentTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataDocument* document = static_cast<const GeoDataDocument*>(node);

    for (const GeoDataStyle::ConstPtr& style : document->styles())
    {
        writeElement(style.data(), writer);
    }

    for (const GeoDataStyleMap& map : document->styleMaps())
    {
        writeElement(&map, writer);
    }

    for (const GeoDataSchema& schema : document->schemas())
    {
        writeElement(&schema, writer);
    }

    QVector<GeoDataFeature*>::ConstIterator it =  document->constBegin();
    QVector<GeoDataFeature*>::ConstIterator const end = document->constEnd();

    for (; it != end; ++it)
    {
        writeElement(*it, writer);
    }

    return true;
}

} // namespace Marble
