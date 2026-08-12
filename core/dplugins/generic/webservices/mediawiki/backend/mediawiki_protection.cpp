/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Vincent Garcia <xavier dot vincent dot garcia at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_protection.h"

// C++ includes

#include <algorithm>

namespace MediaWiki
{

class Q_DECL_HIDDEN Protection::Private
{
public:

    QString type;
    QString level;
    QString expiry;
    QString source;
};

Protection::Protection()
    : d(new Private)
{
}

Protection::~Protection()
{
    delete d;
}

Protection::Protection(const Protection& other)
    : d(new Private(*(other.d)))
{
}

Protection& Protection::operator=(const Protection& other)
{
    *d = *other.d;

    return *this;
}

bool Protection::operator==(const Protection& other) const
{
    return (
            (type()   == other.type())   &&
            (level()  == other.level())  &&
            (expiry() == other.expiry()) &&
            (source() == other.source())
           );
}

void Protection::setType(const QString& type)
{
    d->type = type;
}

QString Protection::type() const
{
    return d->type;
}

void Protection::setLevel(const QString& level)
{
    d->level = level;
}

QString Protection::level() const
{
    return d->level;
}

void Protection::setExpiry(const QString& expiry)
{
    d->expiry = expiry;
}

QString Protection::expiry() const
{
    return d->expiry;
}

void Protection::setSource(const QString& source)
{
    d->source = source;
}

QString Protection::source() const
{
    return d->source;
}

} // namespace MediaWiki
