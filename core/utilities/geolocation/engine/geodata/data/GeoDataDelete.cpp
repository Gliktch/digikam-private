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

#include "GeoDataDelete.h"

// Local includes

#include "GeoDataContainer_p.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataDeletePrivate : public GeoDataContainerPrivate
{
};


GeoDataDelete::GeoDataDelete()
    : GeoDataContainer(new GeoDataDeletePrivate)
{
}

GeoDataDelete::GeoDataDelete(const GeoDataDelete& other)
    : GeoDataContainer(other, new GeoDataDeletePrivate(*other.d_func()))
{
}

GeoDataDelete& GeoDataDelete::operator=(const GeoDataDelete& other)
{
    if (this != &other)
    {
        Q_D(GeoDataDelete);
        *d = *other.d_func();
    }

    return *this;
}

bool GeoDataDelete::operator==(const GeoDataDelete& other) const
{
    return GeoDataContainer::equals(other);
}

bool GeoDataDelete::operator!=(const GeoDataDelete& other) const
{
    return !this->operator==(other);
}

GeoDataDelete::~GeoDataDelete()
{
}

GeoDataFeature* GeoDataDelete::clone() const
{
    return new GeoDataDelete(*this);
}

const char* GeoDataDelete::nodeType() const
{
    return GeoDataTypes::GeoDataDeleteType;
}

} // namespace Marble
