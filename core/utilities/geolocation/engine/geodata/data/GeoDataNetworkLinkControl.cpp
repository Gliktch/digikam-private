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

#include "GeoDataNetworkLinkControl.h"
#include "GeoDataNetworkLinkControl_p.h"

// Local includes

#include "GeoDataAbstractView.h"

namespace Marble
{

GeoDataNetworkLinkControl::GeoDataNetworkLinkControl()
    : GeoDataContainer(new GeoDataNetworkLinkControlPrivate)
{
}

GeoDataNetworkLinkControl::GeoDataNetworkLinkControl(const GeoDataNetworkLinkControl& other)
    : GeoDataContainer(other, new GeoDataNetworkLinkControlPrivate(*other.d_func()))
{
}

GeoDataNetworkLinkControl& GeoDataNetworkLinkControl::operator=(const GeoDataNetworkLinkControl& other)
{
    if (this != &other)
    {
        Q_D(GeoDataNetworkLinkControl);
        *d = *other.d_func();
    }

    return *this;
}


bool GeoDataNetworkLinkControl::operator==(const GeoDataNetworkLinkControl& other) const
{
    Q_D(const GeoDataNetworkLinkControl);
    const GeoDataNetworkLinkControlPrivate* const other_d = other.d_func();

    if (!GeoDataContainer::equals(other) ||
        d->m_minRefreshPeriod != other_d->m_minRefreshPeriod ||
        d->m_maxSessionLength != other_d->m_maxSessionLength ||
        d->m_cookie != other_d->m_cookie ||
        d->m_message != other_d->m_message ||
        d->m_linkName != other_d->m_linkName ||
        d->m_linkDescription != other_d->m_linkDescription ||
        d->m_linkSnippet != other_d->m_linkSnippet ||
        d->m_maxLines != other_d->m_maxLines ||
        d->m_expires != other_d->m_expires ||
        d->m_update != other_d->m_update)
    {
        return false;
    }

    if (!d->m_abstractView && !other_d->m_abstractView)
    {
        return true;
    }

    if ((!d->m_abstractView && other_d->m_abstractView) ||
        (d->m_abstractView && !other_d->m_abstractView))
    {
        return false;
    }

    if (*d->m_abstractView != *other_d->m_abstractView)
    {
        return false;
    }

    return true;
}

bool GeoDataNetworkLinkControl::operator!=(const GeoDataNetworkLinkControl& other) const
{
    return !this->operator==(other);
}

GeoDataNetworkLinkControl::~GeoDataNetworkLinkControl()
{
}

GeoDataFeature* GeoDataNetworkLinkControl::clone() const
{
    return new GeoDataNetworkLinkControl(*this);
}


const char* GeoDataNetworkLinkControl::nodeType() const
{
    Q_D(const GeoDataNetworkLinkControl);
    Q_UNUSED(d);
    return GeoDataTypes::GeoDataNetworkLinkControlType;
}

qreal GeoDataNetworkLinkControl::minRefreshPeriod() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_minRefreshPeriod;
}

void GeoDataNetworkLinkControl::setMinRefreshPeriod(qreal minRefreshPeriod)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_minRefreshPeriod = minRefreshPeriod;
}

qreal GeoDataNetworkLinkControl::maxSessionLength() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_maxSessionLength;
}

void GeoDataNetworkLinkControl::setMaxSessionLength(qreal maxSessionLength)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_maxSessionLength = maxSessionLength;
}

QString GeoDataNetworkLinkControl::cookie() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_cookie;
}

void GeoDataNetworkLinkControl::setCookie(const QString& cookie)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_cookie = cookie;
}

QString GeoDataNetworkLinkControl::message() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_message;
}

void GeoDataNetworkLinkControl::setMessage(const QString& message)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_message = message;
}

QString GeoDataNetworkLinkControl::linkName() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_linkName;
}

void GeoDataNetworkLinkControl::setLinkName(const QString& linkName)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_linkName = linkName;
}

QString GeoDataNetworkLinkControl::linkDescription() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_linkDescription;
}

void GeoDataNetworkLinkControl::setLinkDescription(const QString& linkDescription)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_linkDescription = linkDescription;
}

QString GeoDataNetworkLinkControl::linkSnippet() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_linkSnippet;
}

void GeoDataNetworkLinkControl::setLinkSnippet(const QString& linkSnippet)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_linkSnippet = linkSnippet;
}

int GeoDataNetworkLinkControl::maxLines() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_maxLines;
}

void GeoDataNetworkLinkControl::setMaxLines(int maxLines)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_maxLines = maxLines;
}

QDateTime GeoDataNetworkLinkControl::expires() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_expires;
}

void GeoDataNetworkLinkControl::setExpires(const QDateTime& expires)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_expires = expires;
}

GeoDataUpdate& GeoDataNetworkLinkControl::update()
{
    Q_D(GeoDataNetworkLinkControl);
    return d->m_update;
}

const GeoDataUpdate& GeoDataNetworkLinkControl::update() const
{
    Q_D(const GeoDataNetworkLinkControl);
    return d->m_update;
}

void GeoDataNetworkLinkControl::setUpdate(const GeoDataUpdate& update)
{
    Q_D(GeoDataNetworkLinkControl);
    d->m_update = update;
}

GeoDataAbstractView* GeoDataNetworkLinkControl::networkAbstractView() const
{
    Q_D(const GeoDataNetworkLinkControl);

    return d->m_abstractView;
}

void GeoDataNetworkLinkControl::setNetworkAbstractView(GeoDataAbstractView* abstractView)
{
    Q_D(GeoDataNetworkLinkControl);

    d->m_abstractView = abstractView;
    d->m_abstractView->setParent(this);
}

} // namespace Marble
