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

#include "GeoDataSimpleArrayData.h"

// Qt includes

#include <QVariant>

// Local includes

#include "GeoDataTypes.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataSimpleArrayDataPrivate
{
public:

    GeoDataSimpleArrayDataPrivate()
    {
    }

    QList<QVariant> m_values;
};

GeoDataSimpleArrayData::GeoDataSimpleArrayData()
    : d(new GeoDataSimpleArrayDataPrivate())
{
}

GeoDataSimpleArrayData::GeoDataSimpleArrayData(const GeoDataSimpleArrayData& other)
    : GeoDataObject(other),
      d            (new GeoDataSimpleArrayDataPrivate(*other.d))
{
}

bool GeoDataSimpleArrayData::operator==(const GeoDataSimpleArrayData& other) const
{
    return (equals(other) && (d->m_values == other.d->m_values));
}

bool GeoDataSimpleArrayData::operator!=(const GeoDataSimpleArrayData& other) const
{
    return !this->operator==(other);
}

GeoDataSimpleArrayData::~GeoDataSimpleArrayData()
{
    delete d;
}

int GeoDataSimpleArrayData::size() const
{
    return d->m_values.size();
}

QVariant GeoDataSimpleArrayData::valueAt(int index) const
{
    return d->m_values.at(index);
}

QList< QVariant > GeoDataSimpleArrayData::valuesList() const
{
    return d->m_values;
}

void GeoDataSimpleArrayData::append(const QVariant& value)
{
    d->m_values.append(value);
}

const char* GeoDataSimpleArrayData::nodeType() const
{
    return GeoDataTypes::GeoDataSimpleArrayDataType;
}

void GeoDataSimpleArrayData::pack(QDataStream& stream) const
{
    GeoDataObject::pack(stream);
}

void GeoDataSimpleArrayData::unpack(QDataStream& stream)
{
    GeoDataObject::unpack(stream);
}

} // namespace Marble
