/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-02
 * Description : Cache for Tag information
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagscache.h"

// Qt includes

#include <QMultiHash>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QStringList>
#include <QMap>
#include <QRegularExpression>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbwatch.h"
#include "itempropertiestab.h"
#include "tagproperties.h"

namespace Digikam
{

static bool lessThanForTagShortInfo(const TagShortInfo& first, const TagShortInfo& second)
{
    return (first.id < second.id);
}

static bool lessThanForTagProperty(const TagProperty& first, const TagProperty& second)
{
    return (first.tagId < second.tagId);
}

typedef QList<TagProperty>::const_iterator                            TagPropertiesConstIterator;
typedef QPair<TagPropertiesConstIterator, TagPropertiesConstIterator> TagPropertiesRange;

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN TagsCache::Private
{
public:

    explicit Private(TagsCache* const qq)
        : q(qq)
    {
    }

public:

    volatile bool               initialized             = false;
    volatile bool               needUpdateInfos         = true;
    volatile bool               needUpdateHash          = true;
    volatile bool               needUpdateProperties    = true;
    volatile bool               needUpdateLabelTags     = true;
    volatile bool               changingDB              = false;

    QReadWriteLock              lock;
    QList<TagShortInfo>         infos;
    QMultiHash<QString, int>    nameHash;

    QList<TagProperty>          tagProperties;
    QHash<QString, QList<int> > tagsWithProperty;
    QSet<int>                   internalTags;
    QVector<int>                colorLabelsTags;        ///< index = Label enum, value = tagId.
    QVector<int>                pickLabelsTags;

    TagsCache* const            q                       = nullptr;

public:

    void checkInfos()
    {
        if (needUpdateInfos && initialized)
        {
            QList<TagShortInfo> newInfos = CoreDbAccess().db()->getTagShortInfos();
            QWriteLocker locker(&lock);
            infos                        = newInfos;
            needUpdateInfos              = false;
        }
    }

    void checkNameHash()
    {
        checkInfos();

        if (needUpdateHash && initialized)
        {
            QWriteLocker locker(&lock);
            nameHash.clear();

            for (const TagShortInfo& info : std::as_const(infos))
            {
                nameHash.insert(info.name, info.id);
            }

            needUpdateHash = false;
        }
    }

    void checkProperties()
    {
        if (needUpdateProperties && initialized)
        {
            QList<TagProperty> props = CoreDbAccess().db()->getTagProperties();

            // Ensure not to lock both locks at the same time.

            QWriteLocker locker(&lock);

            needUpdateProperties = false;
            tagProperties        = props;
            tagsWithProperty.clear();

            QLatin1String internalProp = TagsCache::propertyNameDigikamInternalTag();

            for (const TagProperty& property : std::as_const(tagProperties))
            {
                if (property.property == internalProp)
                {
                    internalTags << property.tagId;
                }
            }
        }
    }

    // Remember to call under lock.

    QList<TagShortInfo>::const_iterator find(int id) const
    {
        TagShortInfo info;
        info.id = id;

        // We use the fact that d->infos is sorted by id.

        QList<TagShortInfo>::const_iterator it;
        it = std::lower_bound(infos.constBegin(),
                              infos.constEnd(), info,
                              lessThanForTagShortInfo);

        if ((it == infos.constEnd()) || (info.id < (*it).id))
        {
            return infos.constEnd();
        }

        return it;
    }

    TagPropertiesRange findProperties(int id) const
    {
        TagProperty prop;
        prop.tagId   = id;
        TagPropertiesRange range;
        range.first  = std::lower_bound(tagProperties.begin(), tagProperties.end(), prop, lessThanForTagProperty);
        range.second = std::upper_bound(range.first, tagProperties.end(), prop, lessThanForTagProperty);

        return range;
    }

    inline TagPropertiesConstIterator toNextTag(TagPropertiesConstIterator it) const
    {
        // Increment iterator until the next tagid is reached.

        int currentId = it->tagId;

        for (++it ; it != tagProperties.end() ; ++it)
        {
            if (it->tagId != currentId)
            {
                break;
            }
        }

        return it;
    }

    inline bool compareProperty(const TagPropertiesConstIterator& it,  const QString& property, const QString& value)
    {
        if (value.isNull())
        {
            return (it->property == property);
        }
        else
        {
            return ((it->property == property) && (it->value == value));
        }
    }

    template <typename T>
    inline bool sortedListContains(const QList<T>& list, const T& value)
    {
        return std::binary_search(list.constBegin(), list.constEnd(), value);
    }

    void checkLabelTags()
    {
        if (needUpdateLabelTags && initialized)
        {
            QVector<int> colorTags(NumberOfColorLabels);
            colorTags[NoColorLabel] = q->getOrCreateInternalTag(InternalTagName::colorLabelNone());
            colorTags[RedLabel]     = q->getOrCreateInternalTag(InternalTagName::colorLabelRed());
            colorTags[OrangeLabel]  = q->getOrCreateInternalTag(InternalTagName::colorLabelOrange());
            colorTags[YellowLabel]  = q->getOrCreateInternalTag(InternalTagName::colorLabelYellow());
            colorTags[GreenLabel]   = q->getOrCreateInternalTag(InternalTagName::colorLabelGreen());
            colorTags[BlueLabel]    = q->getOrCreateInternalTag(InternalTagName::colorLabelBlue());
            colorTags[MagentaLabel] = q->getOrCreateInternalTag(InternalTagName::colorLabelMagenta());
            colorTags[GrayLabel]    = q->getOrCreateInternalTag(InternalTagName::colorLabelGray());
            colorTags[BlackLabel]   = q->getOrCreateInternalTag(InternalTagName::colorLabelBlack());
            colorTags[WhiteLabel]   = q->getOrCreateInternalTag(InternalTagName::colorLabelWhite());

            QVector<int> pickTags(NumberOfPickLabels);
            pickTags[NoPickLabel]   = q->getOrCreateInternalTag(InternalTagName::pickLabelNone());
            pickTags[RejectedLabel] = q->getOrCreateInternalTag(InternalTagName::pickLabelRejected());
            pickTags[PendingLabel]  = q->getOrCreateInternalTag(InternalTagName::pickLabelPending());
            pickTags[AcceptedLabel] = q->getOrCreateInternalTag(InternalTagName::pickLabelAccepted());

            QWriteLocker locker(&lock);
            needUpdateLabelTags     = false;
            colorLabelsTags         = colorTags;
            pickLabelsTags          = pickTags;
        }
    }

    // Remember to call under lock.

    QString tagPath(int id, LeadingSlashPolicy slashPolicy) const
    {
        QString path;
        QList<TagShortInfo>::const_iterator it;

        for (it = find(id) ; it != infos.constEnd() ; it = find(it->pid))
        {
            if (path.isNull())
            {
                path = it->name;
            }
            else
            {
                if (it->name.contains(QLatin1String("_Digikam_root_tag_")))
                {
                    continue;
                }
                else
                {
                    path = it->name + QLatin1Char('/') + path;
                }
            }
        }

        if (slashPolicy == IncludeLeadingSlash)
        {
            path.prepend(QLatin1Char('/'));
        }

        return path;
    }

    QList<int> tagsForFragment(bool (QString::*stringFunction)(const QString&, Qt::CaseSensitivity cs) const,
                               const QString& fragment,
                               Qt::CaseSensitivity caseSensitivity,
                               HiddenTagsPolicy hiddenTagsPolicy)
    {
        checkNameHash();
        QMultiMap<QString, int> idsMap;
        QMultiHash<QString, int>::const_iterator it;
        const bool excludeHiddenTags = (hiddenTagsPolicy == NoHiddenTags);

        if (excludeHiddenTags)
        {
            checkProperties();
        }

        QReadLocker locker(&lock);

        for (it = nameHash.constBegin() ; it != nameHash.constEnd() ; ++it)
        {
            if (
                (!excludeHiddenTags || !internalTags.contains(it.value())) &&
                (it.key().*stringFunction)(fragment, caseSensitivity)
               )
            {
                idsMap.insert(it.key(), it.value());
            }
        }

        return idsMap.values();
    }
};

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ChangingDB
{
public:

    explicit ChangingDB(TagsCache::Private* const dd)
        : d(dd)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

    TagsCache::Private* const d = nullptr;
};

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN TagsCacheCreator
{
public:

    TagsCache object;
};

Q_GLOBAL_STATIC(TagsCacheCreator, creator)

// ------------------------------------------------------------------------------------------

TagsCache::TagsCache()
    : QObject(),
      d      (new Private(this))
{
}

TagsCache::~TagsCache()
{
    delete d;
}

TagsCache* TagsCache::instance()
{
    return &creator->object;
}

void TagsCache::initialize()
{
    if (d->initialized)
    {
        return;
    }

    connect(CoreDbAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
            this, SLOT(slotTagChanged(TagChangeset)),
            Qt::DirectConnection);

    d->initialized = true;
}

void TagsCache::invalidate()
{
    QWriteLocker locker(&d->lock);

    d->needUpdateInfos      = true;
    d->needUpdateHash       = true;
    d->needUpdateProperties = true;
    d->needUpdateLabelTags  = true;

    d->infos.clear();
    d->nameHash.clear();
    d->internalTags.clear();
    d->tagProperties.clear();
    d->pickLabelsTags.clear();
    d->colorLabelsTags.clear();
    d->tagsWithProperty.clear();
}

QLatin1String TagsCache::tagPathOfDigikamInternalTags(LeadingSlashPolicy slashPolicy)
{
    if (slashPolicy == IncludeLeadingSlash)
    {
        return QLatin1String("/_Digikam_Internal_Tags_");
    }
    else
    {
        return QLatin1String("_Digikam_Internal_Tags_");
    }
}

QLatin1String TagsCache::propertyNameDigikamInternalTag()
{
    // Do not change, is written to users' databases.

    return QLatin1String("internalTag");
}

QLatin1String TagsCache::propertyNameExcludedFromWriting()
{
    // Do not change, is written to users' databases.

    return QLatin1String("noMetadataTag");
}

QString TagsCache::tagName(int id) const
{
    d->checkInfos();

    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it = d->find(id);

    if (it != d->infos.constEnd())
    {
        return it->name;
    }

    return QString();
}

QStringList TagsCache::tagNames(const QList<int>& ids, HiddenTagsPolicy hiddenTagsPolicy) const
{
    QStringList names;

    if (!ids.isEmpty())
    {
        for (int id : std::as_const(ids))
        {
            if ((hiddenTagsPolicy == IncludeHiddenTags) || !isInternalTag(id))
            {
                names << tagName(id);
            }
        }
    }

    return names;
}

QString TagsCache::tagPath(int id, LeadingSlashPolicy slashPolicy) const
{
    d->checkInfos();

    QString path;
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it;

    for (it = d->find(id) ; it != d->infos.constEnd() ; it = d->find(it->pid))
    {
        if (path.isNull())
        {
            path = it->name;
        }
        else
        {
            if ((it->name).contains(QRegularExpression(QLatin1String("(_Digikam_root_tag_/|"
                                                                     "/_Digikam_root_tag_|"
                                                                     "_Digikam_root_tag_)"))))
            {
                continue;
            }
            else
            {
                path = it->name + QLatin1Char('/') + path;
            }
        }
    }

    if (slashPolicy == IncludeLeadingSlash)
    {
        path.prepend(QLatin1Char('/'));
    }

    return path;
}

QStringList TagsCache::tagPaths(const QList<int>& ids, LeadingSlashPolicy slashPolicy,
                                HiddenTagsPolicy hiddenTagsPolicy) const
{
    QStringList paths;

    if (!ids.isEmpty())
    {
        for (int id : std::as_const(ids))
        {
            if ((hiddenTagsPolicy == IncludeHiddenTags) || !isInternalTag(id))
            {
                paths << tagPath(id, slashPolicy);
            }
        }
    }

    return paths;
}

QList<int> TagsCache::tagsForName(const QString& tagName, HiddenTagsPolicy hiddenTagsPolicy) const
{
    d->checkNameHash();

    if (hiddenTagsPolicy == NoHiddenTags)
    {
        d->checkProperties();
        QList<int> ids;
        QReadLocker locker(&d->lock);
        QMultiHash<QString, int>::const_iterator it;

        for (it = d->nameHash.constFind(tagName) ; (it != d->nameHash.constEnd()) && (it.key() == tagName) ; ++it)
        {
            if (!d->internalTags.contains(it.value()))
            {
                ids << it.value();
            }
        }

        return ids;
    }

    QReadLocker locker(&d->lock);

    return d->nameHash.values(tagName);
}

int TagsCache::tagForName(const QString& tagName, int parentId) const
{
    d->checkNameHash();
    QReadLocker locker(&d->lock);

    QList<TagShortInfo>::const_iterator tag;
    const auto ids = d->nameHash.values(tagName);

    for (int id : ids)
    {
        tag = d->find(id);

        if (tag == d->infos.constEnd())
        {
            continue;    // Error.
        }

        if (tag->pid == parentId)
        {
            return tag->id;
        }
    }

    return 0;
}

bool TagsCache::hasTag(int id) const
{
    d->checkInfos();
    QReadLocker locker(&d->lock);

    return d->find(id) != d->infos.constEnd();
}

int TagsCache::parentTag(int id) const
{
    d->checkInfos();
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator tag = d->find(id);

    if (tag != d->infos.constEnd())
    {
        return tag->pid;
    }

    return 0;
}

QList<int> TagsCache::parentTags(int id) const
{
    d->checkInfos();

    QList<int> ids;
    QReadLocker locker(&d->lock);
    QList<TagShortInfo>::const_iterator it;

    for (it = d->find(id) ; (it != d->infos.constEnd()) && (it->pid != 0) ;
         it = d->find(it->pid))
    {
        ids.prepend(it->pid);
    }

    return ids;
}

int TagsCache::tagForPath(const QString& path) const
{
    QString fullPath = path;

    if (fullPath.startsWith(QLatin1Char('/')))
    {
        fullPath.remove(0, 1);
    }

    // Split the path into its components.

    QStringList tagHierarchy = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);

    if (tagHierarchy.isEmpty())
    {
        return 0;
    }

    d->checkNameHash();
    QReadLocker locker(&d->lock);

    // The last entry in the list is the leaf node tag name, we use this
    // to lookup all the tag ids with that name, then find the one
    // with a matching full path.

    int tagID                       = 0;
    QString tagnm                   = tagHierarchy.last();
    const QList<int> possibleTagIds = d->nameHash.values(tagnm);

    for (int const id : possibleTagIds)
    {
        if (d->tagPath(id, NoLeadingSlash) == fullPath)
        {
            tagID = id;
            break;
        }
    }

    // We only have one entry in tagHierarchy (no path),
    // if only a tag exists, we can assign it to this.
    // Otherwise we recreate it in the root.

    if (
        (tagID == 0)               &&
        (tagHierarchy.size() == 1) &&
        (possibleTagIds.size() == 1)
       )
    {
        tagID = possibleTagIds.first();
    }

    return tagID;
}

QList<int> TagsCache::tagsForPaths(const QStringList& tagPaths) const
{
    QList<int> ids;

    if (!tagPaths.isEmpty())
    {
        for (const QString& path : std::as_const(tagPaths))
        {
            ids << tagForPath(path);
        }
    }

    return ids;
}

int TagsCache::createTag(const QString& tagPathToCreate)
{
    // Split full tag "url" into list of single tag names.

    QStringList tagHierarchy = tagPathToCreate.split(QLatin1Char('/'), Qt::SkipEmptyParts);

    if (tagHierarchy.isEmpty())
    {
        return 0;
    }

    d->checkNameHash();

    int  tagID                 = 0;
    bool parentTagExisted      = true;
    int parentTagIDForCreation = 0;
    QStringList tagsToCreate;

    {
        int  parentTagID = 0;
        QReadLocker locker(&d->lock);

        // Traverse hierarchy from top to bottom.

        for (const QString& tagnm : std::as_const(tagHierarchy))
        {
            tagID = 0;

            // If the parent tag did not exist, we need not check if the child exists.

            if (parentTagExisted)
            {
                QList<TagShortInfo>::const_iterator tag;

                // Find the tag with tag name according to tagHierarchy,
                // and parent ID identical to the ID of the tag we found in
                // the previous run.

                const auto ids = d->nameHash.values(tagnm);

                for (int id : ids)
                {
                    tag = d->find(id);

                    if ((tag != d->infos.constEnd()) && (tag->pid == parentTagID))
                    {
                        tagID = tag->id;
                        break;
                    }
                }
            }

            if (tagID)
            {
                // Tag already found in DB.

                parentTagID      = tagID;
                parentTagExisted = true;

                continue;
            }
            else
            {
                tagsToCreate << tagnm;

                if (parentTagExisted)
                {
                    parentTagIDForCreation = parentTagID;
                }

                parentTagID      = 0;
                parentTagExisted = false;
            }
        }
    }

    {
        CoreDbAccess access;

        for (const QString& tagnm : std::as_const(tagsToCreate))
        {
            tagID = access.db()->addTag(parentTagIDForCreation, tagnm, QString(), 0);

            if (tagID == -1)
            {
                // Something wrong with DB.

                break;
            }
            else
            {
                // Change signals may be queued within a transaction. We know it changed.

                d->needUpdateInfos = true;
                d->needUpdateHash  = true;
            }

            parentTagIDForCreation = tagID;
        }
    }

    return tagID;
}

QList<int> TagsCache::createTags(const QStringList& tagPaths)
{
    QList<int> ids;

    if (!tagPaths.isEmpty())
    {
        for (const QString& path : std::as_const(tagPaths))
        {
            ids << createTag(path);
        }
    }

    return ids;
}

QList<int> TagsCache::getOrCreateTags(const QStringList& tagPaths)
{
    QList<int> ids;

    if (!tagPaths.isEmpty())
    {
        for (const QString& path : std::as_const(tagPaths))
        {
            ids << getOrCreateTag(path);
        }
    }

    return ids;
}

int TagsCache::getOrCreateTag(const QString& tagPath)
{
    int id = tagForPath(tagPath);

    if (!id)
    {
        id = createTag(tagPath);

        // There is a weakness when createTag is called concurrently.
        // The attempt coming second to CoreDbAccess will fail, but the tag was created
        // So at least try again finding the tag read-only.

        if (id == -1)
        {
            id = tagForPath(tagPath);
        }
    }

    return id;
}

int TagsCache::getOrCreateTagWithProperty(const QString& tagPath, const QString& property, const QString& value)
{
    int tagId = getOrCreateTag(tagPath);

    if (!hasProperty(tagId, property, value))
    {
        TagProperties props(tagId);
        props.setProperty(property, value);
    }

    return tagId;
}

bool TagsCache::hasProperty(int tagId, const QString& property, const QString& value) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    TagPropertiesRange range = d->findProperties(tagId);

    for (TagPropertiesConstIterator it = range.first ; it != range.second ; ++it)
    {
        if (d->compareProperty(it, property, value))
        {
            return true;
        }
    }

    return false;
}

QString TagsCache::propertyValue(int tagId, const QString& property) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    TagPropertiesRange range = d->findProperties(tagId);

    for (TagPropertiesConstIterator it = range.first ; it != range.second ; ++it)
    {
        if (it->property == property)
        {
            return it->value;
        }
    }

    return QString();
}

QStringList TagsCache::propertyValues(int tagId, const QString& property) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    TagPropertiesRange range = d->findProperties(tagId);
    QStringList values;

    for (TagPropertiesConstIterator it = range.first ; it != range.second ; ++it)
    {
        if (it->property == property)
        {
            // The list is ordered by property, after id.

            for ( ; (it != range.second) && (it->property == property) ; ++it)
            {
                values << it->value;
            }

            return values;
        }
    }

    return values;
}

QMap<QString, QString> TagsCache::properties(int tagId) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    QMap<QString, QString> map;
    TagPropertiesRange range = d->findProperties(tagId);

    for (TagPropertiesConstIterator it = range.first ; it != range.second ; ++it)
    {
        map[it->property] = it->value;
    }

    return map;
}

QList<int> TagsCache::tagsWithProperty(const QString& property, const QString& value) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);
    QList<int>  ids;

    for (TagPropertiesConstIterator it = d->tagProperties.constBegin() ; it != d->tagProperties.constEnd() ; )
    {
        // Sort out invalid entries, see bug #277169.

        if (it->tagId <= 0)
        {
            ++it;
            continue;
        }

        if (d->compareProperty(it, property, value))
        {
            ids << it->tagId;
            it = d->toNextTag(it);
        }
        else
        {
            ++it;
        }
    }

    return ids;
}

QList<int> TagsCache::tagsWithPropertyCached(const QString& property) const
{
    d->checkProperties();
    {
        QReadLocker locker(&d->lock);
        QHash<QString, QList<int> >::const_iterator it;
        it = d->tagsWithProperty.constFind(property);

        if (it != d->tagsWithProperty.constEnd())
        {
            return it.value();
        }
    }

    QList<int> tags = tagsWithProperty(property);

    {
        QWriteLocker locker(&d->lock);
        d->tagsWithProperty[property] = tags;
    }

    return tags;
}

bool TagsCache::isInternalTag(int tagId) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);

    return d->internalTags.contains(tagId);
}

QList<int> TagsCache::publicTags(const QList<int>& tagIds) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);

    QList<int>::const_iterator it, it2;

    for (it = tagIds.begin() ; it != tagIds.end() ; ++it)
    {
        if (d->internalTags.contains(*it))
        {
            break;
        }
    }

    if (it == tagIds.end())
    {
        return tagIds;
    }

    QList<int> publicIds;
    publicIds.reserve(it - tagIds.begin());

    // Copy to the point of the first internal tag.

    for (it2 = tagIds.begin() ; it2 != it ; ++it2)
    {
        publicIds << *it2;
    }

    // Continue filtering.

    for ( ; it2 != tagIds.end() ; ++it2)
    {
        if (!d->internalTags.contains(*it2))
        {
            publicIds << *it2;
        }
    }

    return publicIds;
}

bool TagsCache::containsPublicTags(const QList<int>& tagIds) const
{
    d->checkProperties();
    QReadLocker locker(&d->lock);

    for (int id : std::as_const(tagIds))
    {
        if (!d->internalTags.contains(id))
        {
            return true;
        }
    }

    return false;
}

bool TagsCache::canBeWrittenToMetadata(int tagId) const
{
    // As long as we always call isInternalTag first, no need to call checkProperties() again.
/*
    d->checkProperties();
*/
    if (isInternalTag(tagId))
    {
        return false;
    }

    if (d->sortedListContains(tagsWithPropertyCached(propertyNameExcludedFromWriting()), tagId))
    {
        return false;
    }

    return true;
}

int TagsCache::getOrCreateInternalTag(const QString& tagName)
{
    // Ensure the parent tag exists, including the internal property.

    getOrCreateTagWithProperty(tagPathOfDigikamInternalTags(IncludeLeadingSlash), propertyNameDigikamInternalTag());
    QString path = tagPathOfDigikamInternalTags(IncludeLeadingSlash) + QLatin1Char('/') + tagName;

    return getOrCreateTagWithProperty(path, propertyNameDigikamInternalTag());
}

void TagsCache::slotTagChanged(const TagChangeset& changeset)
{
    if (changeset.operation() == TagChangeset::Deleted)
    {
        QString name = this->tagName(changeset.tagId());

        Q_EMIT tagAboutToBeDeleted(name);
    }

    if (!d->changingDB && (changeset.operation() != TagChangeset::IconChanged))
    {
        invalidate();
    }

    if      (changeset.operation() == TagChangeset::Added)
    {
        Q_EMIT tagAdded(changeset.tagId());
    }
    else if (changeset.operation() == TagChangeset::Deleted)
    {
        Q_EMIT tagDeleted(changeset.tagId());
    }
}

int TagsCache::tagForColorLabel(int label)
{
    if ((label < FirstColorLabel) || (label > LastColorLabel))
    {
        return 0;
    }

    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    return d->colorLabelsTags[label];
}

QVector<int> TagsCache::colorLabelTags()
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    return d->colorLabelsTags;
}

int TagsCache::colorLabelForTag(int tagId)
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    return d->colorLabelsTags.indexOf(tagId);
}

int TagsCache::colorLabelFromTags(const QList<int>& tagIds)
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    for (int tagId : std::as_const(tagIds))
    {
        for (int i = FirstColorLabel ; i <= LastColorLabel ; ++i)
        {
            if (d->colorLabelsTags[i] == tagId)
            {
                return i;
            }
        }
    }

    return (-1);
}

int TagsCache::tagForPickLabel(int label)
{
    if ((label < FirstPickLabel) || (label > LastPickLabel))
    {
        return 0;
    }

    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    return d->pickLabelsTags[label];
}

QVector<int> TagsCache::pickLabelTags()
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    return d->pickLabelsTags;
}

int TagsCache::pickLabelForTag(int tagId)
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    return d->pickLabelsTags.indexOf(tagId);
}

int TagsCache::pickLabelFromTags(const QList<int>& tagIds)
{
    d->checkLabelTags();
    QReadLocker locker(&d->lock);

    for (int tagId : std::as_const(tagIds))
    {
        for (int i = FirstPickLabel ; i <= LastPickLabel ; ++i)
        {
            if (d->pickLabelsTags[i] == tagId)
            {
                return i;
            }
        }
    }

    return (-1);
}

QStringList TagsCache::shortenedTagPaths(const QList<int>& ids,
                                         QList<int>* sortedIds,
                                         LeadingSlashPolicy slashPolicy,
                                         HiddenTagsPolicy hiddenTagsPolicy) const
{
    QStringList paths;
    QList<QVariant> variantIds;

    // Duplicates tagPath(), but we need the additional list of tag ids.

    for (int id : std::as_const(ids))
    {
        if ((hiddenTagsPolicy == IncludeHiddenTags) || !isInternalTag(id))
        {
            paths      << tagPath(id, slashPolicy);
            variantIds << id;
        }
    }

    // The code is needed in libdigikamcore, so it cannot be moved here. TODO: Find a good place.

    QStringList shortenedPaths = ItemPropertiesTab::shortenedTagPaths(paths, &variantIds);

    for (const QVariant& var : std::as_const(variantIds))
    {
        (*sortedIds) << var.toInt();
    }

    return shortenedPaths;
}

QStringList TagsCache::shortenedTagPaths(const QList<int>& ids,
                                         LeadingSlashPolicy slashPolicy,
                                         HiddenTagsPolicy hiddenTagsPolicy) const
{
    return ItemPropertiesTab::shortenedTagPaths(tagPaths(ids, slashPolicy, hiddenTagsPolicy));
}

QList<int> TagsCache::tagsStartingWith(const QString& fragment, Qt::CaseSensitivity caseSensitivity,
                                       HiddenTagsPolicy hiddenTagsPolicy)
{
    return d->tagsForFragment(&QString::startsWith, fragment, caseSensitivity, hiddenTagsPolicy);
}

QList<int> TagsCache::tagsContaining(const QString& fragment, Qt::CaseSensitivity caseSensitivity,
                                     HiddenTagsPolicy hiddenTagsPolicy)
{
    return d->tagsForFragment(&QString::contains, fragment, caseSensitivity, hiddenTagsPolicy);
}

} // namespace Digikam

#include "moc_tagscache.cpp"
