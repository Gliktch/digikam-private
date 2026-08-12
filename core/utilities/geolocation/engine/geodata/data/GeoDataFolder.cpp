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

#include "GeoDataFolder.h"

// Local includes

#include "GeoDataTypes.h"
#include "GeoDataContainer_p.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataFolderPrivate : public GeoDataContainerPrivate
{
};


GeoDataFolder::GeoDataFolder()
    : GeoDataContainer(new GeoDataFolderPrivate)
{
}

GeoDataFolder::GeoDataFolder(const GeoDataFolder& other)
    : GeoDataContainer(other, new GeoDataFolderPrivate(*other.d_func()))
{
}

GeoDataFolder::~GeoDataFolder()
{
}

GeoDataFolder& GeoDataFolder::operator=(const GeoDataFolder& other)
{
    if (this != &other)
    {
        Q_D(GeoDataFolder);
        *d = *other.d_func();
    }

    return *this;
}

bool GeoDataFolder::operator==(const GeoDataFolder& other) const
{
    return GeoDataContainer::equals(other);
}

bool GeoDataFolder::operator!=(const GeoDataFolder& other) const
{
    return !this->operator==(other);
}

const char* GeoDataFolder::nodeType() const
{
    return GeoDataTypes::GeoDataFolderType;
}

GeoDataFeature* GeoDataFolder::clone() const
{
    return new GeoDataFolder(*this);
}

} // namespace Marble
