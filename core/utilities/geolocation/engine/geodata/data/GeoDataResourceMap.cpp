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

#include "GeoDataResourceMap.h"

// Local includes

#include "GeoDataAlias.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataResourceMapPrivate
{
public:

    GeoDataAlias m_alias;

    GeoDataResourceMapPrivate();
};

GeoDataResourceMapPrivate::GeoDataResourceMapPrivate() :
    m_alias()
{
    // nothing to do
}

GeoDataResourceMap::GeoDataResourceMap() : d(new GeoDataResourceMapPrivate)
{
    // nothing to do
}

GeoDataResourceMap::GeoDataResourceMap(const Marble::GeoDataResourceMap& other) :
    GeoNode(other), d(new GeoDataResourceMapPrivate(*other.d))
{
    // nothing to do
}

GeoDataResourceMap& GeoDataResourceMap::operator=(const GeoDataResourceMap& other)
{
    *d = *other.d;
    return *this;
}

bool GeoDataResourceMap::operator==(const GeoDataResourceMap& other) const
{
    return d->m_alias == other.d->m_alias;
}

bool GeoDataResourceMap::operator!=(const GeoDataResourceMap& other) const
{
    return !this->operator==(other);
}

GeoDataResourceMap::~GeoDataResourceMap()
{
    delete d;
}

const char* GeoDataResourceMap::nodeType() const
{
    return GeoDataTypes::GeoDataResourceMapType;
}

const GeoDataAlias& GeoDataResourceMap::alias() const
{
    return d->m_alias;
}

GeoDataAlias& GeoDataResourceMap::alias()
{
    return d->m_alias;
}

void GeoDataResourceMap::setAlias(const GeoDataAlias& alias)
{
    d->m_alias = alias;
}

QString GeoDataResourceMap::sourceHref() const
{
    return d->m_alias.sourceHref();
}

QString GeoDataResourceMap::targetHref() const
{
    return d->m_alias.targetHref();
}

void GeoDataResourceMap::setSourceHref(const QString& sourceHref)
{
    d->m_alias.setSourceHref(sourceHref);
}

void GeoDataResourceMap::setTargetHref(const QString& targetHref)
{
    d->m_alias.setTargetHref(targetHref);
}

} // namespace Marble
