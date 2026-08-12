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

#include "KmlFolderTagWriter.h"

// Qt includes

#include <QVector>

// Local includes

#include "GeoDataFolder.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "GeoDataTypes.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerDocument(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataFolderType),
                                                                          QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                              new KmlFolderTagWriter());

bool KmlFolderTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataFolder* folder = static_cast<const GeoDataFolder*>(node);

    // Write all containing features

    QVector<GeoDataFeature*>::ConstIterator it =  folder->constBegin();
    QVector<GeoDataFeature*>::ConstIterator const end = folder->constEnd();

    for (; it != end; ++it)
    {
        writeElement(*it, writer);
    }

    return true;
}

KmlFolderTagWriter::KmlFolderTagWriter() :
    KmlFeatureTagWriter(QString::fromUtf8(kml::kmlTag_Folder))
{
    // nothing to do
}

} // namespace Marble
