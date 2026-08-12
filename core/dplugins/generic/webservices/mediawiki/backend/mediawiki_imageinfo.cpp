/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_imageinfo.h"

// C++ includes

#include <algorithm>

namespace MediaWiki
{

class Q_DECL_HIDDEN Imageinfo::Private
{
public:

    QDateTime                timestamp;
    QString                  user;
    QString                  comment;
    QUrl                     url;
    QUrl                     descriptionUrl;
    QUrl                     thumbUrl;
    qint64                   thumbWidth     = -1;
    qint64                   thumbHeight    = -1;
    qint64                   size           = -1;
    qint64                   width          = -1;
    qint64                   height         = -1;
    QString                  sha1;
    QString                  mime;
    QHash<QString, QVariant> metadata;
};

Imageinfo::Imageinfo()
    : d(new Private)
{
}

Imageinfo::Imageinfo(const Imageinfo& other)
    : d(new Private(*(other.d)))
{
}

Imageinfo::~Imageinfo()
{
    delete d;
}

Imageinfo& Imageinfo::operator=(const Imageinfo& other)
{
    *d = *other.d;

    return *this;
}

bool Imageinfo::operator==(const Imageinfo& other) const
{
    return (
            (timestamp()      == other.timestamp())      &&
            (user()           == other.user())           &&
            (comment()        == other.comment())        &&
            (url()            == other.url())            &&
            (descriptionUrl() == other.descriptionUrl()) &&
            (thumbUrl()       == other.thumbUrl())       &&
            (thumbWidth()     == other.thumbWidth())     &&
            (thumbHeight()    == other.thumbHeight())    &&
            (size()           == other.size())           &&
            (width()          == other.width())          &&
            (height()         == other.height())         &&
            (sha1()           == other.sha1())           &&
            (mime()           == other.mime())           &&
            (metadata()       == other.metadata())
           );
}

QDateTime Imageinfo::timestamp() const
{
    return d->timestamp;
}

void Imageinfo::setTimestamp(const QDateTime& timestamp)
{
    d->timestamp = timestamp;
}

QString Imageinfo::user() const
{
    return d->user;
}

void Imageinfo::setUser(const QString& user)
{
    d->user = user;
}

QString Imageinfo::comment() const
{
    return d->comment;
}

void Imageinfo::setComment(const QString& comment)
{
    d->comment = comment;
}

QUrl Imageinfo::url() const
{
    return d->url;
}

void Imageinfo::setUrl(const QUrl& url)
{
    d->url = url;
}

QUrl Imageinfo::descriptionUrl() const
{
    return d->url;
}

void Imageinfo::setDescriptionUrl(const QUrl& descriptionUrl)
{
    d->descriptionUrl = descriptionUrl;
}

QUrl Imageinfo::thumbUrl() const
{
    return d->thumbUrl;
}

void Imageinfo::setThumbUrl(const QUrl& thumbUrl)
{
    d->thumbUrl = thumbUrl;
}

qint64 Imageinfo::thumbWidth() const
{
    return d->thumbWidth;
}

void Imageinfo::setThumbWidth(qint64 thumbWidth)
{
    d->thumbWidth = thumbWidth;
}

qint64 Imageinfo::thumbHeight() const
{
    return d->thumbHeight;
}

void Imageinfo::setThumbHeight(qint64 thumbHeight)
{
    d->thumbHeight = thumbHeight;
}

qint64 Imageinfo::size() const
{
    return d->size;
}

void Imageinfo::setSize(qint64 size)
{
    d->size = size;
}

qint64 Imageinfo::width() const
{
    return d->width;
}

void Imageinfo::setWidth(qint64 width)
{
    d->width = width;
}

qint64 Imageinfo::height() const
{
    return d->height;
}

void Imageinfo::setHeight(qint64 height)
{
    d->height = height;
}

QString Imageinfo::sha1() const
{
    return d->sha1;
}

void Imageinfo::setSha1(const QString& sha1)
{
    d->sha1 = sha1;
}

QString Imageinfo::mime() const
{
    return d->mime;
}

void Imageinfo::setMime(const QString& mime)
{
    d->mime = mime;
}

const QHash<QString, QVariant>& Imageinfo::metadata() const
{
    return d->metadata;
}

QHash<QString, QVariant>& Imageinfo::metadata()
{
    return d->metadata;
}

void Imageinfo::setMetadata(const QHash<QString, QVariant>& metadata)
{
     d->metadata = metadata;
}

} // namespace MediaWiki
