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

#include "GeoDataNetworkLink.h"

// Local includes

#include "GeoDataTypes.h"
#include "GeoDataLink.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataNetworkLinkPrivate
{
public:

    bool        m_refreshVisibility;

    bool        m_flyToView;

    GeoDataLink m_link;

    GeoDataNetworkLinkPrivate();
};

GeoDataNetworkLinkPrivate::GeoDataNetworkLinkPrivate() :
    m_refreshVisibility(false), m_flyToView(false)
{
    // nothing to do
}

GeoDataNetworkLink::GeoDataNetworkLink() : d(new GeoDataNetworkLinkPrivate)
{
    // nothing to do
}

GeoDataNetworkLink::GeoDataNetworkLink(const Marble::GeoDataNetworkLink& other) :
    GeoDataFeature(other), d(new GeoDataNetworkLinkPrivate(*other.d))
{
    // nothing to do
}

GeoDataNetworkLink& GeoDataNetworkLink::operator=(const GeoDataNetworkLink& other)
{
    GeoDataFeature::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataNetworkLink::operator==(const GeoDataNetworkLink& other) const
{
    return equals(other) &&
           d->m_refreshVisibility == other.d->m_refreshVisibility &&
           d->m_link == other.d->m_link &&
           d->m_flyToView == other.d->m_flyToView;
}

bool GeoDataNetworkLink::operator!=(const GeoDataNetworkLink& other) const
{
    return !this->operator==(other);
}

GeoDataNetworkLink::~GeoDataNetworkLink()
{
    delete d;
}

GeoDataFeature* GeoDataNetworkLink::clone() const
{
    return new GeoDataNetworkLink(*this);
}


const char* GeoDataNetworkLink::nodeType() const
{
    return GeoDataTypes::GeoDataNetworkLinkType;
}

bool GeoDataNetworkLink::refreshVisibility() const
{
    return d->m_refreshVisibility;
}

void GeoDataNetworkLink::setRefreshVisibility(bool refreshVisibility)
{
    d->m_refreshVisibility = refreshVisibility;
}

bool GeoDataNetworkLink::flyToView() const
{
    return d->m_flyToView;
}

void GeoDataNetworkLink::setFlyToView(bool flyToView)
{
    d->m_flyToView = flyToView;
}

GeoDataLink& GeoDataNetworkLink::link()
{
    return d->m_link;
}

const GeoDataLink& GeoDataNetworkLink::link() const
{
    return d->m_link;
}

void GeoDataNetworkLink::setLink(const GeoDataLink& link)
{
    d->m_link = link;
}

} // namespace Marble
