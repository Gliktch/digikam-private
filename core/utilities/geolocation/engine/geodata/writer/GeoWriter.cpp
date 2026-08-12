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

#include "GeoWriter.h"

// Local includes

#include "GeoDocument.h"
#include "GeoTagWriter.h"
#include "KmlElementDictionary.h"
#include "DgmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

GeoWriter::GeoWriter()
    :     m_documentType(QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)) // FIXME: work out a standard way to do this.
{
}

bool GeoWriter::write(QIODevice* device, const GeoNode* feature)
{
    setDevice(device);
    setAutoFormatting(true);
    writeStartDocument();

    // FIXME: write the starting tags. Possibly register a tag handler to do this
    // with a null string as the object name?

    GeoTagWriter::QualifiedName name(QString(), m_documentType);
    const GeoTagWriter* writer = GeoTagWriter::recognizes(name);

    if (writer)
    {
        // FIXME is this too much of a hack?

        writer->write(/* node = */ nullptr, *this); // node is never used in write()
    }
    else
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "There is no GeoWriter registered for: " << name;

        return false;
    }

    if (! writeElement(feature))
    {
        return false;
    }

    // close the document

    writeEndElement();

    return true;
}

bool GeoWriter::writeElement(const GeoNode* object)
{
    // Add checks to see that everything is ok here

    GeoTagWriter::QualifiedName name(QString::fromUtf8(object->nodeType()), m_documentType);
    const GeoTagWriter* writer = GeoTagWriter::recognizes(name);

    if (writer)
    {
        if (! writer->write(object, *this))
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "An error has been reported by the GeoWriter for: "
                                           << name;
            return false;
        }
    }
    else
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "There is no GeoWriter registered for: " << name;

        return true;
    }

    return true;
}


void GeoWriter::setDocumentType(const QString& documentType)
{
    m_documentType = documentType;
}

void GeoWriter::writeElement(const QString& namespaceUri, const QString& key, const QString& value)
{
    writeStartElement(namespaceUri, key);
    writeCharacters(value);
    writeEndElement();
}

void GeoWriter::writeElement(const QString& key, const QString& value)
{
    writeStartElement(key);
    writeCharacters(value);
    writeEndElement();
}

void GeoWriter::writeOptionalElement(const QString& key, const QString& value, const QString& defaultValue)
{
    if (value != defaultValue)
    {
        writeElement(key, value);
    }
}

void GeoWriter::writeOptionalAttribute(const QString& key, const QString& value, const QString& defaultValue)
{
    if (value != defaultValue)
    {
        writeAttribute(key, value);
    }
}

} // namespace Marble
