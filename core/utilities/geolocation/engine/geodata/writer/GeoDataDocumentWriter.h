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

#pragma once

// qt includes

#include <QIODevice>
#include <QSet>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class GeoWriterBackend;
class GeoDataDocument;

/**
 * Write GeoDataDocument content to I/O devices (e.g. a file)
 */
class DIGIKAM_EXPORT GeoDataDocumentWriter
{
public:

    /**
     * Write the content of the given GeoDataDocument to the given I/O device
     * @param device An I/O device open for writing
     * @param document A GeoDataDocument with content to write
     * @param documentIdentifier XML document identifier or file extension that determines
     * @return True if writing is successful, false otherwise
     */
    static bool write(QIODevice* device, const GeoDataDocument& document, const QString& documentIdentifier);

    /**
     * Convenience method that uses a QFile as QIODevice and determines the document type from the filename extension
     * @param filename Target file's name
     * @param document Document to write
     * @param documentIdentifier XML document identifier or filename extension that determines the content type.
     * Use an empty string (default) to determine it automatically
     * @return True if writing is successful, false otherwise
     */
    static bool write(const QString& filename, const GeoDataDocument& document, const QString& documentIdentifier = QString());

    /**
     * Registers a plugin as a backend for writing a certain file extension. Usually called by the MARBLE_ADD_WRITER macro.
     * @param writer Backend to register
     * @param fileExtension File extension to associate the backend with
     */
    static void registerWriter(GeoWriterBackend* writer, const QString& fileExtension);
    static void unregisterWriter(GeoWriterBackend* writer, const QString& fileExtension);

private:

    static QString determineDocumentIdentifier(const QString& filename);

private:

    static QSet<QPair<QString, GeoWriterBackend*> > s_backends;
};

} // namespace Marble
