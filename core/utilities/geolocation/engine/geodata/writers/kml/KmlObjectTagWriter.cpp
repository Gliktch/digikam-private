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

#include "KmlObjectTagWriter.h"

// Local includes

#include "GeoWriter.h"

namespace Marble
{

void KmlObjectTagWriter::writeIdentifiers(GeoWriter& writer, const GeoDataObject* object)
{
    if (object && !object->id().isEmpty())
    {
        writer.writeAttribute(QLatin1String("id"),
                              object->id());
    }

    if (object && !object->targetId().isEmpty())
    {
        writer.writeAttribute(QLatin1String("targetId"),
                              object->targetId());
    }
}

} // namespace Marble
