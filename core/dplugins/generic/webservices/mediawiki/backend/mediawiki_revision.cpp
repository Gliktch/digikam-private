/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Robin Bussenot <bussenot dot robin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_revision.h"

// C++ includes

#include <algorithm>

namespace MediaWiki
{

class Q_DECL_HIDDEN Revision::Private
{
public:

    int       revId         = -1;
    int       parentId      = -1;
    int       size          = -1;
    bool      minorRevision = false;
    QString   user;
    QDateTime timestamp;
    QString   comment;
    QString   content;
    QString   parseTree;
    QString   rollback;
};

Revision::Revision()
    : d(new Private)
{
}

Revision::~Revision()
{
    delete d;
}

Revision::Revision( const Revision& other)
    : d(new Private(*(other.d)))
{
}

Revision& Revision::operator=(const Revision& other)
{
    *d = *other.d;

    return *this;
}

bool Revision::operator==(const Revision& other) const
{
    return (
            (timestamp()     == other.timestamp())     &&
            (user()          == other.user())          &&
            (comment()       == other.comment())       &&
            (content()       == other.content())       &&
            (size()          == other.size())          &&
            (minorRevision() == other.minorRevision()) &&
            (parseTree()     == other.parseTree())     &&
            (parentId()      == other.parentId())      &&
            (rollback()      == other.rollback())      &&
            (revisionId()    == other.revisionId())
           );
}

void Revision::setRevisionId(int revisionId)
{
    d->revId = revisionId;
}

int Revision::revisionId() const
{
    return d->revId;
}

void Revision::setParentId(int parentId)
{
    d->parentId = parentId;
}

int Revision::parentId() const
{
    return d->parentId;
}

void Revision::setSize(int size)
{
    d->size = size;
}

int Revision::size() const
{
    return d->size;
}

void Revision::setMinorRevision(bool minorRevision)
{
    d->minorRevision = minorRevision;
}
bool Revision::minorRevision() const
{
    return d->minorRevision;
}

QDateTime Revision::timestamp() const
{
    return d->timestamp;
}

void Revision::setTimestamp(const QDateTime& timestamp)
{
    d->timestamp = timestamp;
}

QString Revision::user() const
{
    return d->user;
}

void Revision::setUser(const QString& user)
{
    d->user = user;
}

void Revision::setComment(const QString& com)
{
    d->comment = com;
}

QString Revision::comment() const
{
    return d->comment;
}

QString Revision::content() const
{
    return d->content;
}

void Revision::setContent(const QString& content)
{
    d->content = content;
}

void Revision::setParseTree(const QString& parseTree)
{
    d->parseTree = parseTree;
}

QString Revision::parseTree() const
{
    return d->parseTree;
}

void Revision::setRollback(const QString& rollback)
{
    d->parseTree = rollback;
}

QString Revision::rollback() const
{
    return d->rollback;
}

} // namespace MediaWiki
