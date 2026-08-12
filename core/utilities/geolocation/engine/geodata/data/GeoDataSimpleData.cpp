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

#include "GeoDataSimpleData.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataSimpleDataPrivate
{
public:

    QString m_name;
    QString m_data;
};

GeoDataSimpleData::GeoDataSimpleData()
    : d(new GeoDataSimpleDataPrivate)
{
}

GeoDataSimpleData::GeoDataSimpleData(const GeoDataSimpleData& other)
    : d(new GeoDataSimpleDataPrivate(*other.d))
{
}

GeoDataSimpleData::~GeoDataSimpleData()
{
    delete d;
}

QString GeoDataSimpleData::name() const
{
    return d->m_name;
}

void GeoDataSimpleData::setName(const QString& name)
{
    d->m_name = name;
}

QString GeoDataSimpleData::data() const
{
    return d->m_data;
}

void GeoDataSimpleData::setData(const QString& data)
{
    d->m_data = data;
}

GeoDataSimpleData& GeoDataSimpleData::operator=(const GeoDataSimpleData& rhs)
{
    *d = *rhs.d;
    return *this;
}

bool GeoDataSimpleData::operator==(const GeoDataSimpleData& other) const
{
    return d->m_name == other.d->m_name &&
           d->m_data == other.d->m_data;
}

bool GeoDataSimpleData::operator!=(const GeoDataSimpleData& other) const
{
    return !this->operator==(other);
}

const char* GeoDataSimpleData::nodeType() const
{
    return GeoDataTypes::GeoDataSimpleDataType;
}

void GeoDataSimpleData::pack(QDataStream& stream) const
{
    stream << d->m_name;
    stream << d->m_data;
}

void GeoDataSimpleData::unpack(QDataStream& stream)
{
    stream >> d->m_name;
    stream >> d->m_data;
}

} // namespace Marble
