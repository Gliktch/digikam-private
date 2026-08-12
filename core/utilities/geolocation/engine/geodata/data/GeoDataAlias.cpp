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

#include "GeoDataAlias.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataAliasPrivate
{
public:

    QString m_sourceHref;
    QString m_targetHref;

    GeoDataAliasPrivate();
};

GeoDataAliasPrivate::GeoDataAliasPrivate() :
    m_sourceHref(), m_targetHref()
{
    // nothing to do
}

GeoDataAlias::GeoDataAlias() : d(new GeoDataAliasPrivate)
{
    // nothing to do
}

GeoDataAlias::GeoDataAlias(const Marble::GeoDataAlias& other) :
    GeoNode(other), d(new GeoDataAliasPrivate(*other.d))
{
    // nothing to do
}

GeoDataAlias& GeoDataAlias::operator=(const GeoDataAlias& other)
{
    *d = *other.d;
    return *this;
}

bool GeoDataAlias::operator==(const GeoDataAlias& other) const
{
    return (d->m_sourceHref == other.d->m_sourceHref) && (d->m_targetHref == other.d->m_targetHref);
}

bool GeoDataAlias::operator!=(const GeoDataAlias& other) const
{
    return !this->operator==(other);
}

GeoDataAlias::~GeoDataAlias()
{
    delete d;
}

const char* GeoDataAlias::nodeType() const
{
    return GeoDataTypes::GeoDataAliasType;
}

QString GeoDataAlias::sourceHref() const
{
    return d->m_sourceHref;
}

QString GeoDataAlias::targetHref() const
{
    return d->m_targetHref;
}

void GeoDataAlias::setSourceHref(const QString& sourceHref)
{
    d->m_sourceHref = sourceHref;
}

void GeoDataAlias::setTargetHref(const QString& targetHref)
{
    d->m_targetHref = targetHref;
}

} // namespace Marble
