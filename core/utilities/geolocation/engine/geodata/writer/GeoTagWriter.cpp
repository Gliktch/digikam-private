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

#include "GeoTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "digikam_debug.h"

namespace Marble
{

GeoTagWriter::GeoTagWriter()
{
}

GeoTagWriter::~GeoTagWriter()
{
}

bool GeoTagWriter::writeElement(const GeoNode* object,
                                GeoWriter& writer)
{
    return writer.writeElement(object);
}

void GeoTagWriter::registerWriter(const QualifiedName& name,
                                  const GeoTagWriter* writer)
{
    TagHash* tagHash = tagWriterHash();

    if (tagHash->contains(name))
    {
        qCWarning(DIGIKAM_GEOENGINE_LOG) << "Warning: The tag"
                                         << name
                                         << "is already registered. Often this indicates that multiple versions of the "
                                            "marblewidget library are loaded at the same time. This will likely lead to problems. "
                                            "Please check your installation, especially internal Marble plugins and external "
                                            "applications that install Marble plugins.";
    }

    else
    {
        tagHash->insert(name, writer);
    }

    Q_ASSERT(tagHash->contains(name));
}

void GeoTagWriter::unregisterWriter(const GeoTagWriter::QualifiedName& qName)
{
    auto hash = tagWriterHash();

    Q_ASSERT(hash->contains(qName));

    delete hash->value(qName);
    hash->remove(qName);

    Q_ASSERT(!hash->contains(qName));
}

GeoTagWriter::TagHash* GeoTagWriter::tagWriterHash()
{
    static TagHash s_tagWriterHash;

    return &s_tagWriterHash;
}

const GeoTagWriter* GeoTagWriter::recognizes(const QualifiedName& qname)
{
    const TagHash* hash = tagWriterHash();

    if (!hash->contains(qname))
    {
        return nullptr;
    }

    return hash->value(qname);
}

} // namespace Marble
