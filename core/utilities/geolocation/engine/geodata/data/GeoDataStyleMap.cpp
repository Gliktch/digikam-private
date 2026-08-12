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

#include "GeoDataStyleMap.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataStyleMapPrivate
{
public:

    QString lastKey;
};

GeoDataStyleMap::GeoDataStyleMap()
    : d(new GeoDataStyleMapPrivate)
{
}

GeoDataStyleMap::GeoDataStyleMap(const GeoDataStyleMap& other)
    : GeoDataStyleSelector(other),
      QMap<QString, QString>(other),
      d(new GeoDataStyleMapPrivate(*other.d))
{
}

GeoDataStyleMap::~GeoDataStyleMap()
{
    delete d;
}

const char* GeoDataStyleMap::nodeType() const
{
    return GeoDataTypes::GeoDataStyleMapType;
}

QString GeoDataStyleMap::lastKey() const
{
    return d->lastKey;
}

void GeoDataStyleMap::setLastKey(const QString& key)
{
    d->lastKey = key;
}

GeoDataStyleMap& GeoDataStyleMap::operator=(const GeoDataStyleMap& other)
{
    QMap<QString, QString>::operator=(other);
    GeoDataStyleSelector::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataStyleMap::operator==(const GeoDataStyleMap& other) const
{
    if (GeoDataStyleSelector::operator!=(other)
        /*
                 PORT_QT6
                 || QMap<QString, QString>::operator!=( other )
        */
       )
    {
        return false;
    }

    return d->lastKey == other.d->lastKey;
}

bool GeoDataStyleMap::operator!=(const GeoDataStyleMap& other) const
{
    return !this->operator==(other);
}

void GeoDataStyleMap::pack(QDataStream& stream) const
{
    GeoDataStyleSelector::pack(stream);
    // lastKey doesn't need to be stored as it is needed at runtime only
    stream << *this;
}

void GeoDataStyleMap::unpack(QDataStream& stream)
{
    GeoDataStyleSelector::unpack(stream);

    stream >> *this;
}

} // namespace Marble
