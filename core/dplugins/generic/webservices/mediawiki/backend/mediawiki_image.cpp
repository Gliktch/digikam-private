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
 * SPDX-FileCopyrightText: 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_image.h"

// C++ includes

#include <algorithm>

namespace MediaWiki
{

class Q_DECL_HIDDEN Image::Private
{
public:

    Private() = default;

    qint64  namespaceId = -1;
    QString title;
};

Image::Image()
    : d(new Private)
{
}

Image::Image(const Image& other)
    : d(new Private(*(other.d)))
{
}

Image::~Image()
{
    delete d;
}

Image& Image::operator=(const Image& other)
{
    *d = *other.d;

    return *this;
}

bool Image::operator==(const Image& other) const
{
    return (
            (namespaceId() == other.namespaceId()) &&
            (title()       == other.title())
           );
}

qint64 Image::namespaceId() const
{
    return d->namespaceId;
}

void Image::setNamespaceId(qint64 namespaceId)
{
    d->namespaceId = namespaceId;
}

QString Image::title() const
{
    return d->title;
}

void Image::setTitle(const QString& title)
{
    d->title = title;
}

} // namespace MediaWiki
