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

#include "OsmRunner.h"

// Local includes

#include "GeoDataDocument.h"
#include "OsmParser.h"
#include "digikam_debug.h"

namespace Marble
{

OsmRunner::OsmRunner(QObject* parent)
    : ParsingRunner(parent)
{
}

GeoDataDocument* OsmRunner::parseFile(const QString& fileName, DocumentRole role, QString& error)
{
    GeoDataDocument* document = OsmParser::parse(fileName, error);

    if (document)
    {
        document->setDocumentRole(role);
        document->setFileName(fileName);
    }

    return document;
}

} // namespace Marble

#include "moc_OsmRunner.cpp"
