/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Remi Benoit <r3m1 dot benoit at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_usergroup.h"

// C++ includes

#include <algorithm>

namespace MediaWiki
{

class Q_DECL_HIDDEN UserGroup::Private
{
public:

    unsigned int   number = -1;

    QString        name;

    QList<QString> rights;
};

UserGroup::UserGroup()
    : d(new Private)
{
}

UserGroup::UserGroup(const UserGroup& other)
    : d(new Private(*(other.d)))
{
}

UserGroup::~UserGroup()
{
    delete d;
}

UserGroup& UserGroup::operator=(const UserGroup& other)
{
    *d = *other.d;

    return *this;
}

bool UserGroup::operator==(const UserGroup& other) const
{
    return (
            (number() == other.number()) &&
            (rights() == other.rights()) &&
            (name()   == other.name())
           );
}

QString UserGroup::name() const
{
    return d->name;
}

void UserGroup::setName(const QString& name)
{
    d->name = name;
}

const QList<QString>& UserGroup::rights() const
{
    return d->rights;
}

QList<QString>& UserGroup::rights()
{
    return d->rights;
}

void UserGroup::setRights(const QList<QString>& rights)
{
    d->rights = rights;
}

qint64 UserGroup::number() const
{
    return d->number;
}

void UserGroup::setNumber(qint64 number)
{
    d->number = number;
}

} // namespace MediaWiki
