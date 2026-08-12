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

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

class QIODevice;

namespace Marble
{

class GeoDataDocument;

class DIGIKAM_EXPORT GeoWriterBackend
{
public:

    virtual ~GeoWriterBackend();

    /**
     * Write the contents of the given document to the device. This method is to be implemented by plugins.
     * @param device An I/O device open for writing
     * @param document A GeoDataDocument with content to write
     * @return True if the content is successfully written to the device, false otherwise
     */
    virtual bool write(QIODevice* device, const GeoDataDocument& document) = 0;
};

/**
 * Helper class for writer backend registration. This class is commonly used through the MARBLE_ADD_WRITER macro
 */
class DIGIKAM_EXPORT GeoWriterBackendRegistrar
{
public:

    GeoWriterBackendRegistrar(GeoWriterBackend* writer, const QString& fileExtension);
    ~GeoWriterBackendRegistrar();

private:

    GeoWriterBackend* m_writer = nullptr;
    QString m_fileExtension;
};

#ifndef STATIC_BUILD
#   define MARBLE_ADD_WRITER(Class, fileExtension) \
    static GeoWriterBackendRegistrar s_##Class##Registrar(new Class, fileExtension);
#else
#   define MARBLE_ADD_WRITER(Class, fileExtension)
#endif

} // namespace Marble
