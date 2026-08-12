/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-05
 * Description : Factory to create instances of Lookup backends
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lookupfactory.h"

// Local includes

#include "lookupaltitudegeonames.h"

namespace Digikam
{

LookupAltitude* LookupFactory::getAltitudeLookup(const QString& backendName,
                                                 QObject* const parent)
{
    if (backendName == QLatin1String("geonames"))
    {
        return new LookupAltitudeGeonames(parent);
    }

    return nullptr;
}

} // namespace Digikam
