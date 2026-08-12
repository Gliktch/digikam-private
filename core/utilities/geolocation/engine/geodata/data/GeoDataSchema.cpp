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

#include "GeoDataSchema.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "GeoDataTypes.h"
#include "GeoDataSimpleField.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataSchemaPrivate
{
public:

    QHash<QString, GeoDataSimpleField> m_simpleField;
    QString                            m_name;
};

GeoDataSchema::GeoDataSchema()
    : d(new GeoDataSchemaPrivate)
{
}

GeoDataSchema::GeoDataSchema(const QHash<QString, GeoDataSimpleField>& simplefields)
    : d(new GeoDataSchemaPrivate)
{
    d->m_simpleField = simplefields;
}

GeoDataSchema::GeoDataSchema(const GeoDataSchema& other)
    : GeoDataObject(other),
      d(new GeoDataSchemaPrivate(*other.d))
{
}

GeoDataSchema& GeoDataSchema::operator=(const GeoDataSchema& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataSchema::operator==(const GeoDataSchema& other) const
{
    return equals(other) &&
           d->m_name == other.d->m_name &&
           d->m_simpleField == other.d->m_simpleField;
}

bool GeoDataSchema::operator!=(const GeoDataSchema& other) const
{
    return !this->operator==(other);
}

GeoDataSchema::~GeoDataSchema()
{
    delete d;
}

QString GeoDataSchema::schemaName() const
{
    return d->m_name;
}

void GeoDataSchema::setSchemaName(const QString& name)
{
    d->m_name = name;
}

GeoDataSimpleField& GeoDataSchema::simpleField(const QString& name) const
{
    return d->m_simpleField[ name ];
}

void GeoDataSchema::addSimpleField(const GeoDataSimpleField& value)
{
    d->m_simpleField.insert(value.name(), value);
}

QList<GeoDataSimpleField> GeoDataSchema::simpleFields() const
{
    return d->m_simpleField.values();
}

const char* GeoDataSchema::nodeType() const
{
    return GeoDataTypes::GeoDataSchemaType;
}

void GeoDataSchema::pack(QDataStream& stream) const
{
    stream << d->m_simpleField.size();

    QHash<QString, GeoDataSimpleField>::const_iterator begin = d->m_simpleField.constBegin();
    QHash<QString, GeoDataSimpleField>::const_iterator end = d->m_simpleField.constEnd();

    for (; begin != end; ++begin)
    {
        begin.value().pack(stream);
    }
}

void GeoDataSchema::unpack(QDataStream& stream)
{
    int size = 0;
    stream >> size;

    for (int i = 0; i < size; ++i)
    {
        GeoDataSimpleField simpleField;
        simpleField.unpack(stream);
        d->m_simpleField.insert(simpleField.name(), simpleField);
    }
}

} // namespace Marble
