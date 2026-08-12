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

#include "GeoDataTimePrimitive.h"
#include "GeoDataTimePrimitive_p.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

GeoDataTimePrimitive::GeoDataTimePrimitive()
    : GeoDataObject(),
      d(new GeoDataTimePrimitivePrivate)
{
}

GeoDataTimePrimitive::GeoDataTimePrimitive(const GeoDataTimePrimitive& other)
    : GeoDataObject(other),
      d(new GeoDataTimePrimitivePrivate(*other.d))
{
}

GeoDataTimePrimitive::~GeoDataTimePrimitive()
{
    delete d;
}

GeoDataTimePrimitive& GeoDataTimePrimitive::operator=(const GeoDataTimePrimitive& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}

const char* GeoDataTimePrimitive::nodeType() const
{
    return GeoDataTypes::GeoDataTimePrimitiveType;
}

void GeoDataTimePrimitive::pack(QDataStream& stream) const
{
    GeoDataObject::pack(stream);
}

void GeoDataTimePrimitive::unpack(QDataStream& stream)
{
    GeoDataObject::unpack(stream);
}

} // namespace Marble
