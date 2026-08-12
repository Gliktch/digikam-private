/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-05-18
 * Description : Wrapper class for face recognition
 *
 * SPDX-FileCopyrightText: 2013      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "identity.h"

namespace Digikam
{

class Q_DECL_HIDDEN Identity::Private : public QSharedData
{
public:

    Private() = default;

public:

    int                         id = -1;
    QMultiMap<QString, QString> attributes;
};

Identity::Identity()
    : d(new Private)
{
}

Identity::Identity(const Identity& other)
    : d(other.d)
{
}

Identity& Identity::operator=(const Identity& other)
{
    d = other.d;

    return *this;
}

Identity::~Identity()
{
}

bool Identity::isNull() const
{
    return (d->id == -1);
}

bool Identity::operator==(const Identity& other) const
{
    return (d->id == other.d->id);
}

int Identity::id() const
{
    return d->id;
}

void Identity::setId(int id)
{
    d->id = id;
}

QString Identity::attribute(const QString& att) const
{
    return d->attributes.value(att);
}

void Identity::setAttribute(const QString& att, const QString& val)
{
    d->attributes.insert(att, val);
}

void Identity::deleteAttributeValue(const QString& att, const QString& val)
{
    d->attributes.remove(att, val);
}

QMultiMap<QString, QString> Identity::attributesMap() const
{
    return d->attributes;
}

void Identity::setAttributesMap(const QMultiMap<QString, QString>& attributes)
{
    d->attributes = attributes;
}

} // namespace Digikam
