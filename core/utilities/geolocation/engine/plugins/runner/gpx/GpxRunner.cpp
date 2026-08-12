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

#include "GpxRunner.h"

// Qt includes

#include <QFile>

// Local includes

#include "GeoDataDocument.h"
#include "GpxParser.h"
#include "digikam_debug.h"

namespace Marble
{

GpxRunner::GpxRunner(QObject* parent)
    : ParsingRunner(parent)
{
}

GpxRunner::~GpxRunner()
{
}

GeoDataDocument* GpxRunner::parseFile(const QString& fileName, DocumentRole role, QString& error)
{
    QFile file(fileName);

    if (!file.exists())
    {
        error = QStringLiteral("File %1 does not exist").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    // Open file in right mode

    if (!file.open(QIODevice::ReadOnly))
    {
        error = QStringLiteral("File %1 cannot be open").arg(fileName);
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    GpxParser parser;

    if (!parser.read(&file))
    {
        error = parser.errorString();
        qCWarning(DIGIKAM_GEOENGINE_LOG) << error;

        return nullptr;
    }

    GeoDocument* const document = parser.releaseDocument();
    Q_ASSERT(document);
    GeoDataDocument* const doc = static_cast<GeoDataDocument*>(document);
    doc->setDocumentRole(role);
    doc->setFileName(fileName);

    file.close();

    return doc;
}

} // namespace Marble

#include "moc_GpxRunner.cpp"
