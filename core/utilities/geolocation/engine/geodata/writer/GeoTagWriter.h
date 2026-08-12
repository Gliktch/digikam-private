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

#include <QPair>
#include <QHash>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class GeoNode;
class GeoWriter;

/**
 * @brief Base class intended to be subclassed by specific XML tag writers
 * This class provides a base class that allows the writing of many different
 * XML formats. The system used to implement this writing system is very strongly
 * based on the @see GeoTagHandler system.
 */
class DIGIKAM_EXPORT GeoTagWriter
{
public:

    virtual bool write(const GeoNode* node, GeoWriter& writer) const = 0;

    /**
     * @brief Object Name and Namespace Pair
     * This type is intended to be used in a similar way to @see GeoParser::QualifiedName
     * but in practice will act differently. The Namespace will not be an XML
     * namespace directly but instead it will refer to a Document Type so that
     * the GeoWriter will be able to identify what GeoTagWriter to use even in
     * absence of an XML namespace. This also allows for the case where data
     * using an internal representation of the KML classes can be outputted in
     * alternative XML formats. For XML formats that have namespaces this
     * document type will usually correspond with the XML namespace. Use in the
     * order QPair<QString tagName, QString documentType>.
     */
    typedef QPair<QString, QString> QualifiedName;

protected:

    GeoTagWriter();
    virtual ~GeoTagWriter();

    static bool writeElement(const GeoNode* object, GeoWriter& writer);

private:

    // Only our registrar is allowed to register tag writers.

    friend struct GeoTagWriterRegistrar;
    static void registerWriter(const QualifiedName&, const GeoTagWriter*);
    static void unregisterWriter(const QualifiedName&);

private:

    // Collect the Tag Writers and provide a singleton like accessor

    typedef QHash<QualifiedName, const GeoTagWriter*> TagHash;
    static TagHash* tagWriterHash();

private:

    // Only our writer is allowed to access tag handlers.

    friend class GeoWriter;
    friend class GeoDataDocumentWriter;
    static const GeoTagWriter* recognizes(const QualifiedName&);
};

// Helper structure

struct GeoTagWriterRegistrar
{
public:

    GeoTagWriterRegistrar(const GeoTagWriter::QualifiedName& name, const GeoTagWriter* writer)
        : m_name(name)
    {
        GeoTagWriter::registerWriter(name, writer);
    }

    ~GeoTagWriterRegistrar()
    {
        GeoTagWriter::unregisterWriter(m_name);
    }

private:

    GeoTagWriter::QualifiedName m_name;
};

} // namespace Marble
