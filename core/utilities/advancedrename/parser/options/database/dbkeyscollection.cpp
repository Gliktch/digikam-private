/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : a base class for a database keys collection
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbkeyscollection.h"

namespace Digikam
{

DbKeysCollection::DbKeysCollection(const QString& n)
    : name(n)
{
}

QString DbKeysCollection::getValue(const QString& key, ParseSettings& settings)
{
    return getDbValue(key, settings);
}

QString DbKeysCollection::collectionName() const
{
    return name;
}

void DbKeysCollection::addId(const QString& id, const QString& description)
{
    if (id.isEmpty() || description.isEmpty())
    {
        return;
    }

    idsMap.insert(id, description);
}

DbKeyIdsMap DbKeysCollection::ids() const
{
    return idsMap;
}

} // namespace Digikam
