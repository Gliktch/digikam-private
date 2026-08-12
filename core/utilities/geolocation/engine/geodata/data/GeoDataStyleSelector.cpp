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

#include "GeoDataStyleSelector.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataStyleSelectorPrivate
{
};

GeoDataStyleSelector::GeoDataStyleSelector() :
    GeoDataObject(),
    d(nullptr)
{
}

GeoDataStyleSelector::GeoDataStyleSelector(const GeoDataStyleSelector& other) :
    GeoDataObject(other),
    d(nullptr)
{
}

GeoDataStyleSelector::~GeoDataStyleSelector()
{
    delete d;
}

GeoDataStyleSelector& GeoDataStyleSelector::operator=(const GeoDataStyleSelector& other)
{
    GeoDataObject::operator=(other);
    return *this;
}

bool GeoDataStyleSelector::operator==(const GeoDataStyleSelector& other) const
{
    return GeoDataObject::equals(other);
}

bool GeoDataStyleSelector::operator!=(const GeoDataStyleSelector& other) const
{
    return !this->operator==(other);
}

void GeoDataStyleSelector::pack(QDataStream& stream) const
{
    GeoDataObject::pack(stream);
}

void GeoDataStyleSelector::unpack(QDataStream& stream)
{
    GeoDataObject::unpack(stream);
}

} // namespace Marble
