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

#include "GeoDataChange.h"

// Local includes

#include "GeoDataContainer_p.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataChangePrivate : public GeoDataContainerPrivate
{
};


GeoDataChange::GeoDataChange()
    : GeoDataContainer(new GeoDataChangePrivate)
{
}

GeoDataChange::GeoDataChange(const GeoDataChange& other)
    : GeoDataContainer(other, new GeoDataChangePrivate(*other.d_func()))
{
}

GeoDataChange& GeoDataChange::operator=(const GeoDataChange& other)
{
    if (this != &other)
    {
        Q_D(GeoDataChange);
        *d = *other.d_func();
    }

    return *this;
}

bool GeoDataChange::operator==(const GeoDataChange& other) const
{
    return GeoDataContainer::equals(other);
}

bool GeoDataChange::operator!=(const GeoDataChange& other) const
{
    return !this->operator==(other);
}

GeoDataChange::~GeoDataChange()
{
}

GeoDataFeature* GeoDataChange::clone() const
{
    return new GeoDataChange(*this);
}

const char* GeoDataChange::nodeType() const
{
    return GeoDataTypes::GeoDataChangeType;
}

} // namespace Marble
