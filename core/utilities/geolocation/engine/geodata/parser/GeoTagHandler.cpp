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

#include "GeoTagHandler.h"

// Local includes

#include "digikam_debug.h"

namespace Marble
{

// Set to a value greater than 0, to dump tag handlers as they get registered
#define DUMP_TAG_HANDLER_REGISTRATION 0

GeoTagHandler::TagHash* GeoTagHandler::s_tagHandlerHash = nullptr;

GeoTagHandler::GeoTagHandler()
{
}

GeoTagHandler::~GeoTagHandler()
{
}

GeoTagHandler::TagHash* GeoTagHandler::tagHandlerHash()
{
    if (!s_tagHandlerHash)
    {
        s_tagHandlerHash = new TagHash();
    }

    Q_ASSERT(s_tagHandlerHash);
    return s_tagHandlerHash;
}

void GeoTagHandler::registerHandler(const GeoParser::QualifiedName& qName, const GeoTagHandler* handler)
{
    TagHash* hash = tagHandlerHash();

    if (!hash->contains(qName))
    {
        hash->insert(qName, handler);
    }

    Q_ASSERT(hash->contains(qName));

#if DUMP_TAG_HANDLER_REGISTRATION > 0
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "[GeoTagHandler] -> Recognizing" << qName.first << "tag with namespace" << qName.second;
#endif
}

void GeoTagHandler::unregisterHandler(const GeoParser::QualifiedName& qName)
{
    TagHash* hash = tagHandlerHash();

    Q_ASSERT(hash->contains(qName));
    delete hash->value(qName);
    hash->remove(qName);
    Q_ASSERT(!hash->contains(qName));
}

const GeoTagHandler* GeoTagHandler::recognizes(const GeoParser::QualifiedName& qName)
{
    TagHash* hash = tagHandlerHash();

    if (!hash->contains(qName))
    {
        return nullptr;
    }

    return (*hash)[qName];
}

} // namespace Marble
