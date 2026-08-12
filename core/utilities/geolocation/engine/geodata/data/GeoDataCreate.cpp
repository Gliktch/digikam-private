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

#include "GeoDataCreate.h"

// Local includes

#include "GeoDataContainer_p.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataCreatePrivate : public GeoDataContainerPrivate
{
};


GeoDataCreate::GeoDataCreate()
    : GeoDataContainer(new GeoDataCreatePrivate)
{
}

GeoDataCreate::GeoDataCreate(const GeoDataCreate& other)
    : GeoDataContainer(other, new GeoDataCreatePrivate(*other.d_func()))
{
}

GeoDataCreate& GeoDataCreate::operator=(const GeoDataCreate& other)
{
    if (this != &other)
    {
        Q_D(GeoDataCreate);
        *d = *other.d_func();
    }

    return *this;
}

bool GeoDataCreate::operator==(const GeoDataCreate& other) const
{
    return GeoDataContainer::equals(other);
}

bool GeoDataCreate::operator!=(const GeoDataCreate& other) const
{
    return !this->operator==(other);
}

GeoDataCreate::~GeoDataCreate()
{
}

GeoDataFeature* GeoDataCreate::clone() const
{
    return new GeoDataCreate(*this);
}

const char* GeoDataCreate::nodeType() const
{
    return GeoDataTypes::GeoDataCreateType;
}

} // nalespace Marble
