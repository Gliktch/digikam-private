/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : Access to the properties of an Item / Tag pair, i.e., a tag associated to an item
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemtagpair.h"

// Qt includes

#include <QSharedData>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "iteminfo.h"
#include "tagscache.h"

namespace Digikam
{

typedef QExplicitlySharedDataPointer<ItemTagPairPriv> ItemTagPairPrivSharedPointer;

// NOTE: do not use Q_DECL_HIDDEN hidden here to prevent export symbols warnings under Windows.

class ItemTagPairPriv : public QSharedData
{

public:

    static ItemTagPairPrivSharedPointer createGuarded(qlonglong imageId, int tagId);

    ItemTagPairPriv() = default;

    bool isNull() const;
    void init(const ItemInfo& info, int tagId);
    void checkProperties();

public:

    ItemInfo                    info;
    int                         tagId               = -1;
    bool                        isAssigned          = false;
    bool                        propertiesLoaded    = false;
    QMultiMap<QString, QString> properties;
};

// -----------------------------------------------------------------------

class Q_DECL_HIDDEN ItemTagPairPrivSharedNull : public ItemTagPairPrivSharedPointer
{
public:

    ItemTagPairPrivSharedNull()
        : QExplicitlySharedDataPointer<ItemTagPairPriv>(new ItemTagPairPriv)
    {
    }
};

Q_GLOBAL_STATIC(ItemTagPairPrivSharedNull, imageTagPairPrivSharedNull)

// -----------------------------------------------------------------------

ItemTagPairPrivSharedPointer ItemTagPairPriv::createGuarded(qlonglong imageId, int tagId)
{
    if ((imageId <= 0) || (tagId <= 0))
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Attempt to create invalid tag pair image id"
                                      << imageId << "tag id" << tagId;

        return *imageTagPairPrivSharedNull;
    }

    return ItemTagPairPrivSharedPointer(new ItemTagPairPriv);
}

void ItemTagPairPriv::init(const ItemInfo& i, int t)
{
    if (isNull())
    {
        return;
    }

    tagId      = t;
    info       = i;
    isAssigned = info.tagIds().contains(tagId);
}

void ItemTagPairPriv::checkProperties()
{
    if (!isNull() && !propertiesLoaded)
    {
        QList<ImageTagProperty> props = CoreDbAccess().db()->getImageTagProperties(info.id(), tagId);

        for (const ImageTagProperty& p : std::as_const(props))
        {
            properties.insert(p.property, p.value);
        }

        propertiesLoaded = true;
    }
}

bool ItemTagPairPriv::isNull() const
{
    return (this == imageTagPairPrivSharedNull->constData());
}

// -----------------------------------------------------------------------

ItemTagPair::ItemTagPair()
    : d(*imageTagPairPrivSharedNull)
{
}

ItemTagPair::~ItemTagPair()
{
}

ItemTagPair::ItemTagPair(qlonglong imageId, int tagId)
    : d(ItemTagPairPriv::createGuarded(imageId, tagId))
{
    d->init(ItemInfo(imageId), tagId);
}

ItemTagPair::ItemTagPair(const ItemInfo& info, int tagId)
    : d(ItemTagPairPriv::createGuarded(info.id(), tagId))
{
    d->init(info, tagId);
}

ItemTagPair::ItemTagPair(const ItemTagPair& other)
    : d(other.d)
{
}

ItemTagPair& ItemTagPair::operator=(const ItemTagPair& other)
{
    d = other.d;

    return *this;
}

bool ItemTagPair::isNull() const
{
    return (d == *imageTagPairPrivSharedNull);
}

QList<ItemTagPair> ItemTagPair::availablePairs(qlonglong imageId)
{
    return availablePairs(ItemInfo(imageId));
}

QList<ItemTagPair> ItemTagPair::availablePairs(const ItemInfo& info)
{
    QList<ItemTagPair> pairs;

    if (info.isNull())
    {
        return pairs;
    }

    QList<int> tagIds = CoreDbAccess().db()->getTagIdsWithProperties(info.id());

    for (int tid : std::as_const(tagIds))
    {
        pairs << ItemTagPair(info, tid);
    }

    return pairs;
}

qlonglong ItemTagPair::imageId() const
{
    return d->info.id();
}

int ItemTagPair::tagId() const
{
    return d->tagId;
}

bool ItemTagPair::isAssigned() const
{
    return d->isAssigned;
}

void ItemTagPair::assignTag()
{
    if (!d->isNull() && !d->isAssigned)
    {
        d->info.setTag(d->tagId);
        d->isAssigned = true;
    }
}

void ItemTagPair::unAssignTag()
{
    if (!d->isNull() && d->isAssigned)
    {
        d->info.removeTag(d->tagId);
        d->isAssigned = false;
    }
}

bool ItemTagPair::hasProperty(const QString& key) const
{
    d->checkProperties();

    return d->properties.contains(key);
}

bool ItemTagPair::hasAnyProperty(const QStringList& keys) const
{
    d->checkProperties();

    for (const QString& key : std::as_const(keys))
    {
        if (d->properties.contains(key))
        {
            return true;
        }
    }

    return false;
}

bool ItemTagPair::hasValue(const QString& key, const QString& value) const
{
    d->checkProperties();

    return d->properties.contains(key, value);
}

QString ItemTagPair::value(const QString& key) const
{
    d->checkProperties();

    return d->properties.value(key);
}

QStringList ItemTagPair::allValues(const QStringList& keys) const
{
    d->checkProperties();
    QStringList vals;

    for (const QString& key : std::as_const(keys))
    {
        vals << d->properties.values(key);
    }

    return vals;
}

QStringList ItemTagPair::values(const QString& key) const
{
    d->checkProperties();

    return d->properties.values(key);
}

QStringList ItemTagPair::propertyKeys() const
{
    d->checkProperties();

    return d->properties.keys();
}

QMultiMap<QString, QString> ItemTagPair::properties() const
{
    d->checkProperties();

    return d->properties;
}

void ItemTagPair::setProperty(const QString& key, const QString& value)
{
    if (d->isNull() || d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    // for single entries in db, this can of course be optimized using a single UPDATE WHERE

    removeProperties(key);
    d->properties.replace(key, value);
    CoreDbAccess().db()->addImageTagProperty(d->info.id(), d->tagId, key, value);
}

void ItemTagPair::addProperty(const QString& key, const QString& value)
{
    if (d->isNull() || d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    if (!d->properties.contains(key, value))
    {
        d->properties.insert(key, value);
        CoreDbAccess().db()->addImageTagProperty(d->info.id(), d->tagId, key, value);
    }
}

void ItemTagPair::removeProperty(const QString& key, const QString& value)
{
    if (d->isNull() || d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    if (d->properties.contains(key, value))
    {
        CoreDbAccess().db()->removeImageTagProperties(d->info.id(), d->tagId, key, value);
        d->properties.remove(key, value);
    }
}

void ItemTagPair::removeProperties(const QString& key)
{
    if (d->isNull() || d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    if (d->properties.contains(key))
    {
        CoreDbAccess().db()->removeImageTagProperties(d->info.id(), d->tagId, key);
        d->properties.remove(key);
    }
}

void ItemTagPair::clearProperties()
{
    if (d->isNull() || d->info.isNull())
    {
        return;
    }

    if (d->propertiesLoaded && d->properties.isEmpty())
    {
        return;
    }

    CoreDbAccess().db()->removeImageTagProperties(d->info.id(), d->tagId);
    d->properties.clear();
    d->propertiesLoaded = true;
}

} // namespace Digikam
